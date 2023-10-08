#pragma once

#include <zecalculator/math_objects/aliases.h>
#include <zecalculator/parsing/shared.h>

#include <tuple>

namespace zc {

template <parsing::Type type>
using MathObjects = std::tuple<CppUnaryFunction,
                               CppBinaryFunction,
                               GlobalConstant,
                               Function<type, 0>,
                               Function<type, 1>,
                               Function<type, 2>,
                               Sequence<type>>;

}
