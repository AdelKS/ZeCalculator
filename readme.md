### ZeCalculator

`ZeCalculator` is a `C++20` library for parsing and computing mathematical expressions and objects.

#### Goals
The aim of this library is to provide the required features for the software [ZeGrapher](https://github.com/AdelKS/ZeGrapher), which has the following needs:
- Handle elegantly wrong math expressions
  - Give meaningful error messages: what went wrong, on what part of the equation
  - Be failsafe
- Has little dependencies for easy packaging
- Fast repetitive evaluation of math objects, once the user has properly defined every math object.
- Know dependencies between objects (which object calls which other objects)

#### Design requirements
The approach is to implement the concept of a "math world":
- Within a "math world", objects can reach and call each other for querying and evaluation.
- Every instance of `MathWorld` is based on a `default world` that contains the usual functions and constants.
- Have a permanent pointer/reference to a math object and be able to change its definition or type using it
- C++ functions can be added to a world

### Example code

```c++
#include <zecalculator/zecalculator.h>

using namespace zc;
using namespace tl;
using namespace std;

int main()
{
  ast::MathWorld world;

  // Notes about adding a math object to a math world:
  // - Each added object exists only within the math world that creates it
  // - Adding a math object returns a DynMathObject reference that is essentially an expected<variant, error>
  //   with some helper functions.
  //   - the variant contains all the possible objects: function, sequence, global constant, global variable, cpp function
  //   - the error expresses what went wrong in parsing the equation

  // Add a one parameter function named "f"
  // Note that 'my_constant' is only defined later
  // - this function's state will be updated once 'my_constant' is defined
  // - (re)defining objects within a math world can potentially modify every other objects
  ast::DynMathObject& obj1 = world.add("f(x) = x + my_constant + cos(math::pi)");

  // the expected should hold a variant whose alternative is a single variable function
  assert(obj1.holds<ast::Function<1>>());

  // if we try to evaluate the function, we get an Error object
  assert(obj1(1.0).error() == Error::undefined_variable(parsing::tokens::Text("my_constant", 11), "f(x) = x + my_constant + cos(math::pi)"));

  // Add a global constant called "my_constant" with an initial value of 3.0
  ast::DynMathObject& obj2 = world.add("my_constant = 3.0");

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
  world.add("g(z) = 2*z + my_constant");

  // redefine what 'obj1' is
  // - a function of two variables, with different names now
  // - calls the function 'g'
  world.redefine(obj1, "h(u, v) = u + v + my_constant + g(v)");

  // the equation should be parsed as a two-argument function
  assert(obj1.holds<ast::Function<2>>());

  // evaluate function again and get the new value
  assert(obj1(1, 3).value() == 16);

  // ======================================================================================

  // the underlying objects can be retrieved either by using the fact that
  // DynMathObject publicly inherits expected<variant, error> or the 'value_as' helper function:
  // - "value" prefix just like expected::value, i.e. can throw
  // - "value_as" as a wrapper to std::get<>(expected::value), can throw for two different reasons
  //   - the expected has an error
  //   - the alternative asked is not the actual one held by the variant
  ast::Function<2>& func = obj1.value_as<ast::Function<2>>();
  ast::GlobalConstant& my_constant = obj2.value_as<ast::GlobalConstant>();

  // each specific math object has extra public methods that may prove useful

  // functions can be in error state if one of its direct dependencies are undefined
  // e.g. "f(x) = cos(x) + g(x)" but "g(x)" is undefined
  assert(not func.error());

  // We can query the direct (or all) dependencies of Function based objects
  // the methods returns a map that gives the names and the type of dep
  assert(bool(func.direct_dependencies()
              == std::unordered_map{std::pair(std::string("my_constant"), deps::VARIABLE),
                                    {"g", deps::FUNCTION}}));

  // overwrite the value of the global constant
  // without needing to redefine it through a full equation (which will require parsing etc...)
  my_constant = 5.0;

  // Function objects can also be evaluated
  assert(func({1, 1}).value() == 14);

  return 0;
}
```

More examples of what the library can do are in the [test](./test/) folder.

#### Interface

The library is still in its early stages, expect the interface to change frequently. The library can already parse and evaluate math expressions.

The math world is implemented as [zc::MathWorld](./include/zecalculator/mathworld/decl/mathworld.h)
1. Stores its objects in a container of [zc::DynMathObject](./include/zecalculator/math_objects/decl/dyn_math_object.h)
2. Does not invalidate references to existing contained objects when growing/shrinking

when adding an object, the math world returns a [zc::DynMathObject&](./include/zecalculator/math_objects/decl/dyn_math_object.h), and that can be used as a "permanent handle"
1. Publicly inherits `tl::expected<std::variant<...>, zc::Error>`, where `std::variant<...>` contains all the possible math object types supported by the library.
   - Underlying objects can be accessed using the [expected](https://en.cppreference.com/w/cpp/utility/expected) class it inherits or its `value_as` helper method.
2. Can be redefined through its owning math world `zc::MathWorld::redefine(zc::DynMathObject&, std::string equation)`
   - Note: redefining the object has been put in the math world because it can potentially modify all the other objects
3. Can be evaluated using its method `evaluate(double...)` or `operator () (double...)`

Error messages when expressions have faulty syntax or semantics are expressed through the [zc::Error](include/zecalculator/error.h) class:
  - If it is known, gives what part of the equation raised the error.
  - If it is known, gives the type of error.

Two namespaces are offered, that express the underlying representation of the parsed math objects
- `zc::ast`: using the abstract syntax tree representation (AST)
- `zc::rpn`: using reverse polish notation (RPN) / postfix notation in a flat representation in memory.

The RPN representation should have faster evaluation (it is not really the case currently) but is generated from an `ast` representation, therefore it has slower parsing time (TODO: needs a benchmark).

#### Benchmarks
There is for now one benchmark defined in the tests, called "parametric function benchmark" in the file [test/function_test.cpp](test/function_test.cpp), that computes the average evaluation time of the function `f(x) = 3*cos(t*x) + 2*sin(x/t) + 4`, where `t` is a global constant, in `ast` vs `rpn` vs `c++`.

The current results are (AMD Ryzen 5950X, `-march=native -O3` compile flags)
- `g++ 13.2.1` + `libstdc++` + `ld.bfd 2.41.0`
  - `ast`: 145ns ± 5ns
  - `rpn`: 145ns ± 5ns
  - `c++`: 75ns ± 5ns
- `clang++ 17.0.6` + `libc++` + `ld.lld 17.0.6`
  - `ast`: 135ns ± 5ns
  - `rpn`: 150ns ± 5ns
  - `c++`: 75ns ± 5ns

<details>

<summary>Benchmark code snippet</summary>

```c++
  "parametric function benchmark"_test = []<class StructType>()
  {
    {
      constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;
      constexpr std::string_view data_type_str_v = std::is_same_v<StructType, AST_TEST> ? "AST" : "RPN";

      MathWorld<type> world;
      auto& t = world.add("t = 1").template value_as<GlobalConstant<type>>();
      auto& f = world.add("f(x) =3*cos(t*x) + 2*sin(x/t) + 4").template value_as<Function<type, 1>>();

      double x = 0;
      auto begin = high_resolution_clock::now();
      double res = 0;
      size_t iterations = 0;
      while (high_resolution_clock::now() - begin < 1s)
      {
        res += f({x}).value();
        iterations++;
        x++;
        t.set_fast(t.value()+1);
      }
      auto end = high_resolution_clock::now();
      std::cout << "Avg zc::Function<" << data_type_str_v << "> eval time: "
                << duration_cast<nanoseconds>((end - begin) / iterations).count() << "ns"
                << std::endl;
      std::cout << "dummy val: " << res << std::endl;
    }
    {
      double cpp_t = 1;
      auto cpp_f = [&](double x) {
        return 3*cos(cpp_t*x) + 2*sin(x/cpp_t) + 4;
      };

      double x = 0;
      auto begin = high_resolution_clock::now();
      double res = 0;
      size_t iterations = 0;
      while (high_resolution_clock::now() - begin < 1s)
      {
        res += cpp_f(x);
        iterations++;
        x++;
        cpp_t++;
      }
      auto end = high_resolution_clock::now();
      std::cout << "Avg C++ function eval time: " << duration_cast<nanoseconds>((end - begin)/iterations).count() << "ns" << std::endl;
      std::cout << "dummy val: " << res << std::endl;

    }

  } | std::tuple<AST_TEST, RPN_TEST>{};
```

</details>

#### How to build

The project uses the [meson](mesonbuild.com/) build system, to build and run tests:
```shell
git clone https://github.com/AdelKS/ZeCalculator
cd ZeCalculator
meson setup build -D test=true
cd build
meson compile
```

#### How to run tests
Once the library is built (see above), all tests can simply be run with
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
