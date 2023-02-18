### ZeCalculator

`ZeCalculator` is a `C++20` library for parsing and computing mathematical expressions and objects. It is mainly used as a backend for the plotting software [ZeGrapher](https://github.com/AdelKS/ZeGrapher) but can be useful for other projects.

#### Goals
The aim of this library is to provided the required features for the software [ZeGrapher](https://github.com/AdelKS/ZeGrapher), which has the following needs:
- Handle elegantly wrong math expressions
  - Give meaningful error messages
  - Be failsafe
- Has little dependencies for easy packaging
- Faster repetitive evaluation of math objects on a set of points, once the user has properly defined every math object.

#### Design
The approach is to implement the concept of a "math world":
- Within a "math world", objects can reach and call each other for querying and evaluation.
- A math world can be locked -- i.e. objects cannot be modified by the user -- so that evaluation can become faster with less runtime checks & lookups.

#### Implementation details
To try to fulfill the goals described above, the following implementation is targeted:
- Except for testing. Only the C++23 class [expected](https://github.com/TartanLlama/expected) has been embedded to this library as it is very useful for writing lean code.
- Error messages when expressions have faulty syntax or semantics are expressed through the [zc::Error](include/zecalculator/utils/error.h) class:
  - Gives exactly what part of the expression is making issues with a `std::string_view` on the original expression.
  - Gives the type of error that was met, if it is known.
- Flexible evaluation of mathematical expressions, where mathematical objects are looked up by name in the provided "math world".
- Fast evaluation by binding expression to a "math world" where function and variables names are changed to unique integer IDs within the math world.

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
