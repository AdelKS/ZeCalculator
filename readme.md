### ZeCalculator

`ZeCalculator` is a `C++20` library for parsing and computing mathematical expressions and objects.

#### Goals
The aim of this library is to provide the required features for the software [ZeGrapher](https://github.com/AdelKS/ZeGrapher), which has the following needs:
- Handle elegantly wrong math expressions
  - Give meaningful error messages
  - Be failsafe
- Has little dependencies for easy packaging
- Faster repetitive evaluation of math objects, once the user has properly defined every math object.

#### Design goals
The approach is to implement the concept of a "math world":
- [x] Within a "math world", objects can reach and call each other for querying and evaluation.
  - [x] Every instance of `MathWorld` is based on a `default world` that contains the usual functions and constants.
- [x] C++ functions can be added to a world, which makes it extendable.
- [ ]  A math world can be locked -- i.e. objects cannot be modified by the user (except global constants) -- so that evaluation can become faster with less runtime checks & lookups.

#### Current example code

The library is still in its early stages, but it can already parse and evaluate math expressions
```c++
#include <zecalculator/zecalculator.h>

#include <iostream>

using namespace zc;
using namespace tl;
using namespace std;

int main()
{
  ast::MathWorld world;

  // Notes about adding a math object to a math world:
  // - Each added object exists only within the math world that creates it
  // - Adding a math object returns a tl::expected that can have an error instead of the
  //   handle to the function (e.g.: invalid format for the name, or name is already taken)

  // Add a function named "f", note that the constant "my_constant" is only defined after
  // Note: the .value() call from tl::expected<> throws if it actually hold an error
  auto f = world.add<ast::Function>("f", ast::Function({"x"}, "x + my_constant + cos(math::pi)")).value();

  // Add a global constant called "my_constant" with an initial value of 3.0
  // Note: the .value() call from tl::expected<> throws if it actually hold an error
  auto cst = world.add<GlobalConstant>("my_constant", 3.0).value();

  // Evaluate function and get the value
  // Notes:
  // - We know the expression is correct, otherwise the call `.value()` will throw
  // - The error can be recovered with '.error()`
  // - To know if the result is correct
  //   - call `.has_value()`
  //   - use the `bool()` operator on the expression
  std::cout << f({1}).value()  << std::endl; // == 3

  // overwrite the value of the global constant
  *cst = 5.0;

  // evaluate function again and get the new value
  std::cout << f({1}).value()  << std::endl; // == 5

  // change 'f': new variable names and count, call a function g
  *f = Function({"y", "z"}, "y + z + my_constant + g(y)");

  // define function 'g'
  auto g = world.add<Function>("g").value();

  // assign input variables and expression to 'g'
  *g = Function({"z"}, "2*z + my_constant");

  // evaluate function again and get the new value
  std::cout << f({3, 4}).value() << std::endl; // == 23

  return 0;
}
```

More examples of what the library can do are in the [test](./test/) folder.

#### Implementation details
To try to fulfill the goals described above, the following implementation is targeted:
- Except for testing. Only the C++23 class [expected](https://github.com/TartanLlama/expected) has been embedded to this library as it is very useful for writing lean code.
- Error messages when expressions have faulty syntax or semantics are expressed through the [zc::Error](include/zecalculator/error.h) class:
  - Gives exactly what part of the expression is making issues with an [zc::SubstrInfo](include/zecalculator/utils/substr_info.h) instance.
  - Gives the type of error that was met, if it is known.
- Flexible evaluation of mathematical expressions, where mathematical objects are looked up by name in the provided "math world".
- Fast evaluation by binding expression to a "math world" where function and variables names are directly pointed to by reference.


Two namespaces are offered where objects are defined:
- `zc::ast`: using the abstract syntax tree representation (AST)
- `zc::rpn`: using reverese polish notation (RPN) / postfix notation in a flat representation in memory.

The RPN representation has faster evaluation but is generated from an `ast` representation, therefore it has slower parsing time.

#### Benchmarks
There is for now one benchmark defined in the tests, called "parametric function benchmark" in the file [test/function_test.cpp](test/function_test.cpp), that computes the average evaluation time of the function `f(x) = cos(x) + t + 3`, where t is a global constant, in `ast` vs `rpn` vs `c++`.

The current results are (AMD Ryzen 5950X, `gcc 13.2.1`, `-march=native -O3` compile flags)
- `ast`: 175ns
- `rpn`: 140ns
- `c++`: 30ns

There is ongoing work to implement the last design goal with evaluating math objects without name lookups on "frozen" math worlds, which should be the fastest.

<details>

<summary>Benchmark code snippet</summary>

```c++
 "parametric function benchmark"_test = []<class StructType>()
  {
    {
      constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::AST : parsing::RPN;
      constexpr std::string_view data_type_str_v = std::is_same_v<StructType, AST_TEST> ? "AST" : "RPN";

      MathWorld<type> world;
      auto f = world.add("f", Function<type> ({"x"}, "cos(x) + t + 3")).value();
      auto t = world.add("t", GlobalConstant(0)).value();

      double x = 0;
      auto begin = high_resolution_clock::now();
      double res = 0;
      size_t iterations = 0;
      while (high_resolution_clock::now() - begin < 1s)
      {
        res += f({x}).value();
        iterations++;
        x++;
        t->value++;
      }
      auto end = high_resolution_clock::now();
      std::cout << "Avg zc::Function<" << data_type_str_v << "> eval time: "
                << duration_cast<nanoseconds>((end - begin) / iterations).count() << "ns"
                << std::endl;
      std::cout << "dummy val: " << res << std::endl;
    }
    {
      double cpp_t = 0;
      auto cpp_f = [&](double x) {
        return cos(x) + cpp_t + 3;
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
meson setup build
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
