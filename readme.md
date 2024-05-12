### ZeCalculator

`ZeCalculator` is a `C++20` library for parsing and computing mathematical expressions and objects.

#### Features

- Parse and compute math objects defined through a simple equation of the type `<object declaration> = <math expression>`
  - Multi-variable functions e.g. `f(x, y) = cos(x) * sin(y)`
  - Global Variables: functions without arguments, e.g. `my_var = f(1, 1)`
  - Global constants: simple valued e.g. `my_constant = 1.2`
- Handle elegantly wrong math expressions
  - No exceptions (only used when the user does something wrong)
  - Give meaningful error messages: what went wrong, on what part of the equation
- Has little dependencies for easy packaging
- Fast repetitive evaluation of math objects, once the user has properly defined every math object.
- Know dependencies between objects (which object calls which other objects)

### Example code

```c++
#include <zecalculator/zecalculator.h>

using namespace zc;
using namespace tl;
using namespace std;

int main()
{
  rpn::MathWorld world;

  // Notes about adding a math object to a math world:
  // - Each added object exists only within the math world that creates it
  // - Adding a math object returns a DynMathObject reference that is essentially an expected<variant, error>
  //   with some helper functions.
  //   - the variant contains all the possible objects: function, sequence, global constant, cpp function
  //   - the error expresses what went wrong in adding the object / parsing the equation
  rpn::DynMathObject& obj1 = world.new_object();

  // Add a one parameter function named "f"
  // Note that 'my_constant' is only defined later
  // - this function's state will be updated once 'my_constant' is defined
  // - (re)defining objects within a math world can potentially modify every other objects
  obj1 = "f(x) = x + my_constant + cos(math::pi)";

  // We can query the direct dependencies of any object: name, type, and where in the equation
  // Note: only Function and Sequence instances with a valid equation return a non-empty set
  assert(bool(world.direct_dependencies(obj1)
              == deps::Deps{{"my_constant", {deps::Dep::VARIABLE, {11}}},
                            {"cos", {deps::Dep::FUNCTION, {25}}},
                            {"math::pi", {deps::Dep::VARIABLE, {29}}}}));

  // the expected should hold an error since 'my_constant' is undefined at this point
  assert(not obj1.has_value()
         and obj1.error()
               == Error::undefined_variable(parsing::tokens::Text{"my_constant", 11},
                                            "f(x) = x + my_constant + cos(math::pi)"));

  // Add a global constant called "my_constant" with an initial value of 3.0
  rpn::DynMathObject& obj2 = world.new_object();
  obj2 = "my_constant = 3.0";

  // now that 'my_constant' is defined, 'obj1' gets modified to properly hold a function
  // Note that defining an object in the MathWorld may affect any other object
  // -> Redefining objects is NOT thread-safe
  assert(obj1.holds<rpn::Function>());

  // We can evaluate 'obj1'
  // note: we could also do it when 'my_constant' was undefined,
  //       in that case the result would be the same error as above
  expected<double, Error> eval = obj1(1.0);

  // Notes:
  // - We know the expression is correct, otherwise the call `.value()` will throw
  // - The error can be recovered with '.error()`
  // - To know if the result is correct
  //   - call `.has_value()`
  //   - use the `bool()` operator on 'eval'
  assert(eval.value() == 3);

  // add a single argument function 'g' to the world
  world.new_object() = "g(z) = 2*z + my_constant";

  // redefine what 'obj1' using a new equation
  // - Now it's the Fibonacci sequence called 'u'
  // - we can force the parser to parse it as a sequence
  //   - unneeded here, just for demo
  //   - the object will contain an error if the forced parsing fails
  //     - even if the equation is a valid e.g. GlobalConstant expression
  obj1 = As<rpn::Sequence>{"u(n) = 0 ; 1 ; u(n-1) + u(n-2)"};

  // should hold a Sequence now
  assert(obj1.holds<rpn::Sequence>());

  // evaluate function again and get the new value
  assert(obj1(10).value() == 55);

  // ======================================================================================

  // the underlying objects can be retrieved either by using the fact that
  // DynMathObject publicly inherits expected<variant, error> or the 'value_as' helper function:
  // - "value" prefix just like expected::value, i.e. can throw
  // - "value_as" as a wrapper to std::get<>(expected::value), can throw for two different reasons
  //   - the expected has an error
  //   - the alternative asked is not the actual one held by the variant
  [[maybe_unused]] rpn::Sequence& u = obj1.value_as<rpn::Sequence>();
  [[maybe_unused]] GlobalConstant& my_constant = obj2.value_as<GlobalConstant>();

  // each specific math object has extra public methods that may prove useful

  return 0;
}
```

More examples of what the library can do are in the [test](./test/) folder.

#### Interface

Math objects belong to a [zc::MathWorld](./include/zecalculator/mathworld/decl/mathworld.h)
- Within a "math world", objects can "see" each other in their expressions
- Every instance of `MathWorld` is based on a `default world` that contains the usual functions and constants.
- Stores its objects in a container of [zc::DynMathObject](./include/zecalculator/math_objects/decl/dyn_math_object.h)
  - Does not invalidate references to existing contained objects when growing/shrinking
  - When adding an object, the math world returns a [zc::DynMathObject&](./include/zecalculator/math_objects/decl/dyn_math_object.h), and that can be used as a "permanent handle"
- [zc::DynMathObject&](./include/zecalculator/math_objects/decl/dyn_math_object.h)
  - Publicly inherits `tl::expected<std::variant<...>, zc::Error>`, where `std::variant<...>` contains all the possible math object types supported by the library.
     - Underlying objects can be accessed using the [expected](https://en.cppreference.com/w/cpp/utility/expected) class it inherits or its `value_as` helper method.
  - Can be redefined through its owning math world `zc::MathWorld::redefine(zc::DynMathObject&, std::string equation)`
     - Note: redefining the object has been put in the math world because it can potentially modify all the other objects
  - Can be evaluated using its method `evaluate(double...)` or `operator () (double...)`
- C++ functions can be added

Error messages when expressions have faulty syntax or semantics are expressed through the [zc::Error](include/zecalculator/error.h) class:
  - If it is known, gives what part of the equation raised the error.
  - If it is known, gives the type of error.

Two namespaces are offered, that express the underlying representation of the parsed math objects
- `zc::fast`: using the abstract syntax tree representation (AST)
- `zc::rpn`: using reverse polish notation (RPN) / postfix notation in a flat representation in memory.
  - Generated from the `fast` representation, but the time taken by the extra step is negligible (see results of the test "AST/FAST/RPN creation speed")
  - Has faster evaluation

#### Benchmarks
There is for now one benchmark defined in the tests, called "parametric function benchmark" in the file [test/function_test.cpp](test/function_test.cpp), that computes the average evaluation time of the function `f(x) = 3*cos(t*x) + 2*sin(x/t) + 4`, where `t` is a global constant, in `ast` vs `rpn` vs `c++`.

The current results are (AMD Ryzen 5950X, `-march=native -O3` compile flags)
- `g++ 13.2.1` + `libstdc++` + `ld.bfd 2.41.0`
  - `ast`: 270ns ± 5ns
  - `rpn`: 135ns ± 5ns
  - `c++`: 75ns ± 5ns
- `clang++ 17.0.6` + `libc++` + `ld.lld 17.0.6`
  - `ast`: 245ns ± 5ns
  - `rpn`: 140ns ± 5ns
  - `c++`: 75ns ± 5ns

<details>

<summary>Benchmark code snippet</summary>

```c++
"parametric function benchmark"_test = []<class StructType>()
{
  constexpr auto duration = nanoseconds(500ms);
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;
    constexpr std::string_view data_type_str_v = std::is_same_v<StructType, FAST_TEST> ? "FAST" : "RPN";

    MathWorld<type> world;
    auto& t = world.add("t = 1").template value_as<GlobalConstant>();
    auto& f = world.add("f(x) =3*cos(t*x) + 2*sin(x/t) + 4").template value_as<Function<type>>();

    double x = 0;
    double res = 0;
    size_t iterations =
      loop_call_for(duration, [&]{
        res += f(x).value();
        x++;
        t += 1;
    });
    std::cout << "Avg zc::Function<" << data_type_str_v << "> eval time: "
              << duration_cast<nanoseconds>(duration / iterations).count() << "ns"
              << std::endl;
    std::cout << "dummy val: " << res << std::endl;
  }
  {
    double cpp_t = 1;
    auto cpp_f = [&](double x) {
      return 3*cos(cpp_t*x) + 2*sin(x/cpp_t) + 4;
    };

    double x = 0;
    double res = 0;
    size_t iterations =
      loop_call_for(duration, [&]{
        res += cpp_f(x);
        iterations++;
        x++;
        cpp_t++;
    });
    std::cout << "Avg C++ function eval time: " << duration_cast<nanoseconds>(duration/iterations).count() << "ns" << std::endl;
    std::cout << "dummy val: " << res << std::endl;

  }

} | std::tuple<FAST_TEST, RPN_TEST>{};
```

</details>

#### How to build

The project uses the [meson](mesonbuild.com/) build system. Being header-only,
it does not have a shared library to build: downstream projects only need the headers

#### How to run tests

To build tests
```shell
git clone https://github.com/AdelKS/ZeCalculator
cd ZeCalculator
meson setup build -D test=true
cd build
meson compile
```
Once the library is built, all tests can simply be run with
```
meson test
```
in the `build` folder.

#### How to install
Once the library is built (see above), you can install it by running
```
meson install
```
in the `build` folder.
