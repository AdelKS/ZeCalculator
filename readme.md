### ZeCalculator

`ZeCalculator` is a `C++20` library for parsing and computing mathematical expressions and objects.

#### Goals
The aim of this library is to provide the required features for the software [ZeGrapher](https://github.com/AdelKS/ZeGrapher), which has the following needs:
- Handle elegantly wrong math expressions
  - Give meaningful error messages
  - Be failsafe
- Has little dependencies for easy packaging
- Faster repetitive evaluation of math objects, once the user has properly defined every math object.

#### Design
The approach is to implement the concept of a "math world":
- Within a "math world", objects can reach and call each other for querying and evaluation.
  - Every instance of `MathWorld` is based on a `default world` that contains the usual functions and constants.
  - A `global_world` global variable is available for projects that only need one. Evaluation of objects can omit givin a `MathWorld` instance, in which case the global world is assumed.
- C++ functions can be registered within a global world, which makes it extendable.
- A math world can be locked -- i.e. objects cannot be modified by the user (except global constants) -- so that evaluation can become faster with less runtime checks & lookups.

#### Current example code

The library is still in its early stages, but it can already parse and evaluate expressions
```c++
#include <zecalculator/utils/parser.h>
#include <zecalculator/utils/syntax_tree.h>
#include <zecalculator/utils/evaluation.h>

#include <iostream>

using namespace zc;
using namespace tl;
using namespace std;

int main()
{
  MathWorld world;
  world.add_global_constant("my_constant2", 3.0);

  // can use auto instead of fully specifying the types
  expected<vector<Token>, ParsingError> expect_parsing = parse("cos(1) + my_constant2");
  expected<SyntaxTree, ParsingError> expect_node = make_tree(expect_parsing.value());
  expected<double, EvaluationError> res = evaluate(expect_node.value(), world);

  cout << res.value() << endl; // 3.5403

  return 0;
}
```

More examples of what the library can do are in the [test](./test/) folder.

#### Implementation details
To try to fulfill the goals described above, the following implementation is targeted:
- Except for testing. Only the C++23 class [expected](https://github.com/TartanLlama/expected) has been embedded to this library as it is very useful for writing lean code.
- Error messages when expressions have faulty syntax or semantics are expressed through the [zc::ParsingError](include/zecalculator/utils/parsing_error.h) and [zc::EvaluationError](include/zecalculator/utils/evaluation_error.h) classes:
  - Gives exactly what part of the expression is making issues with a `std::string_view` on the original `std::string` expression (owned by the math object).
  - Gives the type of error that was met, if it is known.
- Flexible evaluation of mathematical expressions, where mathematical objects are looked up by name in the provided "math world".
- Fast evaluation by binding expression to a "math world" where function and variables names are directly pointed to by reference.

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
