#pragma once

#include <zecalculator/mathworld.h>
#include <zecalculator/utils/evaluation_error.h>

namespace zc {

/// @brief evaluates a syntax tree using a given math world
tl::expected<double, EvaluationError> evaluate(const SyntaxTree& tree, const MathWorld& world);

}
