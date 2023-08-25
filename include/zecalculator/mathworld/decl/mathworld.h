#pragma once

#include <zecalculator/math_objects/builtin_binary_functions.h>
#include <zecalculator/math_objects/builtin_unary_functions.h>
#include <zecalculator/mathworld/mathworld_template.h>

namespace zc {

template <parsing::Type>
class Function;
class Sequence;
struct GlobalConstant;
class Expression;

using GlobalVariable = Expression;

using MathWorld = MathWorldT<CppUnaryFunction, CppBinaryFunction, GlobalConstant, Function<parsing::AST>, GlobalVariable, Sequence>;

}
