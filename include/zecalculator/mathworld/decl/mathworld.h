#pragma once

#include "zecalculator/parsing/parser.h"
#include <zecalculator/math_objects/builtin_binary_functions.h>
#include <zecalculator/math_objects/builtin_unary_functions.h>
#include <zecalculator/mathworld/mathworld_template.h>

namespace zc {

template <parsing::Type>
class Function;

template <parsing::Type>
class Sequence;

struct GlobalConstant;

template <parsing::Type>
class Expression;

template <parsing::Type type>
using GlobalVariable = Expression<type>;

template <parsing::Type type>
using MathWorld = MathWorldT<CppUnaryFunction,
                             CppBinaryFunction,
                             GlobalConstant,
                             Function<type>,
                             GlobalVariable<type>,
                             Sequence<type>>;

namespace ast {

using MathWorld = MathWorldT<CppUnaryFunction,
                             CppBinaryFunction,
                             GlobalConstant,
                             Function<parsing::AST>,
                             GlobalVariable<parsing::AST>,
                             Sequence<parsing::AST>>;

}
}
