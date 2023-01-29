### ZeCalculator

`ZeCalculator` is a `C++20` library for parsing and computing mathematical expressions and objects. It is mainly used as a backend for the plotting software [ZeGrapher](https://github.com/AdelKS/ZeGrapher) but can be useful for other projects. The aim of this library is to have no other dependency than the standard library (except for testing).

#### How to build and run tests

The project uses the [meson](mesonbuild.com/) build system, to build and run tests:
```shell
git clone https://github.com/AdelKS/ZeCalculator
cd ZeCalculator
meson setup build
cd build
meson compile
meson test
```
