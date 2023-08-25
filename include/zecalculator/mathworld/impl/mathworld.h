#pragma once

#include <zecalculator/math_objects/builtin_binary_functions.h>
#include <zecalculator/math_objects/builtin_unary_functions.h>
#include <zecalculator/math_objects/impl/function.h>
#include <zecalculator/math_objects/impl/sequence.h>
#include <zecalculator/math_objects/impl/expression.h>
#include <zecalculator/math_objects/global_variable.h>
#include <zecalculator/mathworld/mathworld_template.h>

namespace zc {

using MathWorld = MathWorldT<CppUnaryFunction, CppBinaryFunction, GlobalConstant, ast::Function, GlobalVariable, ast::Sequence>;

}
