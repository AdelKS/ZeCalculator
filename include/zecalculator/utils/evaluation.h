#pragma once

#include <zecalculator/mathworld.h>
#include <zecalculator/utils/evaluation_error.h>

namespace zc {

/// @brief evaluates a syntax tree using a given math world
/// @param tree: tree to evaluate
/// @param input_vars: variables that are given as input to the tree, will shadow any variable in the math world
/// @param world: math world (contains functions, global constants... etc)
tl::expected<double, EvaluationError> evaluate(const SyntaxTree& tree,
                                               const name_map<double>& input_vars,
                                               const MathWorld& world,
                                               size_t current_recursion_depth);

/// @brief evaluates a syntax tree using a given math world
inline tl::expected<double, EvaluationError> evaluate(const SyntaxTree& tree,
                                                      const name_map<double>& input_vars,
                                                      const MathWorld& world)
{
  return evaluate(tree, input_vars, world, 0);
}

/// @brief evaluates a syntax tree using a given math world
inline tl::expected<double, EvaluationError> evaluate(const SyntaxTree& tree, const MathWorld& world)
{
  return evaluate(tree, {}, world, 0);
}


}
