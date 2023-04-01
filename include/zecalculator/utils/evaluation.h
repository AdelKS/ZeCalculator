#pragma once

#include <zecalculator/mathworld.h>
#include <zecalculator/utils/evaluation_error.h>

namespace zc {

/// @brief evaluates a syntax tree using a given math world
tl::expected<double, EvaluationError> evaluate(const SyntaxTree& tree, const MathWorld& world);

/// @brief evaluates a syntax tree using the global math world
inline tl::expected<double, EvaluationError> evaluate(const SyntaxTree& tree)
{
  return evaluate(tree, global_world);
}

}
