### ZeCalculator

`ZeCalculator` is a `C++20` library for parsing and computing mathematical expressions and objects.

#### Features

- Supported math objects
  - defined through a simple equation of the type `<object declaration> = <math expression>`
    - Multi-variable functions e.g. `f(x, y) = cos(x) * sin(y)`
    - Sequences: e.g. `u(n) = 0 ; 1 ; u(n-1) + u(n-2)`
    - Global Variables: functions without arguments, e.g. `my_var = f(1, 1)`
    - Global constants: simple valued e.g. `my_constant = 1.2`
  - (custom) C++ math functions can be added
  - 1D Data with arbitrary expressions (not only just numbers, see code example bellow)
- Handle elegantly wrong math expressions
  - No exceptions (only used when the user does something wrong)
  - Give meaningful error messages: what went wrong, on what part of the equation
- Has no dependencies for easy packaging
- Fast repetitive evaluation of math objects.
- Know dependencies between objects (which object calls which other objects)

#### Example code

```c++
#include <zecalculator/zecalculator.h>
#include <zecalculator/test-utils/print-utils.h>

using namespace zc;
using namespace tl;
using namespace std;

double square(double x) { return x * x; }

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

  // Assign a one parameter function named "f"
  // Note that 'my_constant' is only defined later
  // - this function's state will be updated once 'my_constant' is defined
  // - (re)defining objects within a math world can potentially modify every other objects
  obj1 = "f(x) = x + my_constant + cos(math::pi)";

  // We can query the direct dependencies of any object: name, type, and where in the equation
  // Note: only Function and Sequence instances with a valid equation return a non-empty set
  assert(bool(obj1.direct_dependencies()
              == deps::Deps{{"my_constant", {deps::Dep::VARIABLE, {11}}},
                            {"cos", {deps::Dep::FUNCTION, {25}}},
                            {"math::pi", {deps::Dep::VARIABLE, {29}}}}));

  // the expected should hold an error since 'my_constant' is undefined at this point
  assert(not obj1.has_value()
         and obj1.error()
               == Error::undefined_variable(parsing::tokens::Text{"my_constant", 11},
                                            "f(x) = x + my_constant + cos(math::pi)"));

  rpn::DynMathObject& obj2 = world.new_object();

  // Assign a global constant called "my_constant" with an initial value of 3.0
  obj2 = "my_constant = 3.0";

  // now that 'my_constant' is defined, 'obj1' gets modified to properly hold a function
  // Note that assigning to an object in the MathWorld may affect any other object
  // -> Assigning to objects is NOT thread-safe
  assert(obj1.object_type() == zc::FUNCTION);

  // We can evaluate 'obj1' with an initializer_list<double>
  // note: we could also do it when 'my_constant' was undefined,
  //       in that case the result would be the same error as above
  expected<double, Error> eval = obj1({1.0});

  // Notes:
  // - We know the expression is correct, otherwise the call `.value()` will throw
  // - The error can be recovered with '.error()`
  // - To know if the result is correct
  //   - call `.has_value()`
  //   - use the `bool()` operator on 'eval'
  assert(eval.value() == 3);

  // add a single argument function 'g' to the world
  world.new_object() = "g(z) = 2*z + my_constant";

  // assign a new equation to 'obj1'
  // - Now it's the Fibonacci sequence called 'u'
  //   - Recognized by its "sequence" of expressions
  //     - The last expressions is the "generic" one that applies for any other index
  obj1 = "u(n) = 0 ; 1 ; u(n-1) + u(n-2)";

  // should hold a Sequence now
  assert(obj1.object_type() == zc::SEQUENCE);

  // evaluate function again and get the new value
  assert(obj1({10}).value() == 55);

  // C++ double(double...) functions can also be registered in a world
  auto& obj3 = world.new_object();
  obj3.set("square", CppFunction{square});

  // Can evaluate an expression directly using the math world
  assert(world.evaluate("square(2)").value() == 4.);

  // define Data object
  // can use numbers or complex expressions for each of its values
  // can define a name for the line index, e.g. 'index', so it can be used in its expressions
  auto& obj4 = world.new_object();
  obj4.set_data("data(index)", {"1.0", "square(2)*index", "u(10)"});

  // data objects can be used like regular functions
  // to retrieve their values on each index
  assert(world.evaluate("data(0)").value() == 1.);

  assert(obj4({1}).value() == 4.);
  assert(obj4({2}).value() == 55.);

  obj4.set_data_point(1, "square(3)+1");

  assert(obj4({1}).value() == 10.);

  return 0;
}
```

More examples of what the library can do are in the [test](./test/) folder.

#### Documentation

Classes within header files are fully documented.

Overview:
1. The entry-point class is [MathWorld](./include/zecalculator/mathworld/decl/mathworld.h)
    ```c++
    rpn::MathWorld mathworld;
    ```
   - Within the same `MathWorld` instance, objects can "see" and "talk" to each other.
   - Every instance of `MathWorld` is filled with the usual functions and constants, see [builtin.h](./include/zecalculator/math_objects/builtin.h).
   - Stores its objects in a container of [zc::DynMathObject](./include/zecalculator/math_objects/decl/dyn_math_object.h)
     - Does not invalidate references to unaffected `zc::DynMathObject` instances it contains when growing/shrinking
     - When adding an object, the math world returns a [zc::DynMathObject&](./include/zecalculator/math_objects/decl/dyn_math_object.h), and that can be used as a "permanent handle"
        ```c++
        rpn::DynMathObject& obj = mathworld.new_object();
        ```
2. [DynMathObject](./include/zecalculator/math_objects/decl/dyn_math_object.h) acts as a generic math object
    - Has a "left hand side" (LHS) that defines its name and the name of its input variables, e.g. `  f(x) ` (notice the white spaces)
      - The status of it can be queried with `tl::expected<Ok, zc::Error> name_status() const`
    - Has a "right hand side" (RHS) that defines how the object is compute, and differs among the possible math object types
      - The type can be
        - Simple double valued constant
        - C++ function
        - 1D Data object
        - Sequence
        - Global variable
      - The status of it can be queried with `tl::expected<Ok, zc::Error> object_status() const`
    - The overall status (i.e. valid LHS and RHS) can be queried with
      - `tl::expected<Ok, zc::Error> status() const`
      - `bool has_value()`
      - `std::optional<zc::Error> error()` to retrieve the error in either the LHS or RHS
    - Some extra helper methods
      - `bool holds(ObjectType)` to know what underlying object type is currently set
    - Can be assigned, using `operator =`, equations or math objects.
      - [CppFunction](./include/zecalculator/math_objects/decl/cpp_function.h)
        ```c++
        double square(double x) { return x * x; }
        // ...
        obj = zc::CppFunction{"square", square};
        ```
      - [Function](./include/zecalculator/math_objects/decl/function.h)
        ```c++
        // automatic type deduction
        obj = "f(x) = cos(x)";
        ```
      - [Sequence](./include/zecalculator/math_objects/decl/sequence.h)
        ```c++
        // or sequence with first values, the last expression is the generic expression
        obj = "fibonacci(n) = 0 ; 1 ; fibonacci(n-1) + fibonacci(n-2)"
        ```
      - [Data](./include/zecalculator/math_objects/decl/data.h)
        ```c++
        // requires using set_data()
        obj.set_data("data", {"1.0", "square(2)", "u(10)"});
        ```
      - [GlobalConstant](./include/zecalculator/math_objects/decl/global_constant.h)
        ```c++
        // defined through an equation of type "name = number"
        obj = "pi = 3.14";
        // or assigned directly without the need of parsing
        obj = 3.14;
        ```
    - Can be evaluated
      ```c++
      tl::expected<double, zc::Error> res1 = obj({1.0});
      tl::expected<double, zc::Error> res2 = obj.evaluate({12.0, 3.0});
      ```
3. Error messages when expressions have faulty syntax or semantics are expressed through the [zc::Error](include/zecalculator/error.h) class:
   - If it is known, gives what part of the equation raised the error with the `token` member, of the type [zc::tokens::Text](./include/zecalculator/parsing/data_structures/token.h)
   - If it is known, gives the type of error.
4. Two namespaces are offered, that express the underlying representation of the parsed math objects
   - `zc::fast::`: using the abstract syntax tree representation (AST)
   - `zc::rpn::`: using reverse polish notation (RPN) / postfix notation in a flat representation in memory.
     - Generated from the `fast` representation, but the time taken by the extra step is negligible (see results of the test "AST/FAST/RPN creation speed")
     - Has faster evaluation

#### Benchmarks
There is for now one benchmark defined in the tests, called "parametric function benchmark" in the file [test/function_test.cpp](test/function_test.cpp), that computes the average evaluation time of the function `f(x) = 3*cos(t*x) + 2*sin(x/t) + 4`, where `t` is a global constant, in `ast` vs `rpn` vs `c++`.

The current results are (`-march=native -O3` compile flags)
- `AMD Ryzen 5950X`
  - `g++ 13.2.1` + `libstdc++` + `ld.bfd 2.41.0`
    - `ast`: 270ns ± 5ns
    - `rpn`: 135ns ± 5ns
    - `c++`: 75ns ± 5ns
  - `clang++ 17.0.6` + `libc++` + `ld.lld 17.0.6`
    - `ast`: 245ns ± 5ns
    - `rpn`: 140ns ± 5ns
    - `c++`: 75ns ± 5ns
- `AMD Ryzen 9950X3D`
  - `g++ 15.1.0` + `libstdc++` + `ld.bfd 2.44.0`
    - `ast`: 185ns ± 5ns
    - `rpn`: 110ns ± 5ns
    - `c++`: 60ns ± 5ns
  - `clang++ 20.1.7` + `libc++` + `ld.lld 20.1.7`
    - `ast`: 170ns ± 5ns
    - `rpn`: 115ns ± 5ns
    - `c++`: 60ns ± 5ns

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
    auto& t = (world.new_object() = "t = 1");
    auto& f = (world.new_object() = "f(x) =3*cos(t*x) + 2*sin(x/t) + 4");

    double x = 0;
    double res = 0;
    size_t iterations =
      loop_call_for(duration, [&]{
        res += f({x}).value();
        x++;
        t = x;
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
