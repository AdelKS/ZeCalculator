#pragma once

#include <zecalculator/mathworld.h>
#include <zecalculator/evaluation/error.h>
#include <zecalculator/evaluation/ast/function.h>
#include <zecalculator/evaluation/ast/variable.h>
#include <zecalculator/utils/name_map.h>

namespace zc {

struct NodeEvaluator
{
  const MathWorld& world;
  const name_map<double>& input_vars;
  const size_t current_recursion_depth = 0;


  using ReturnType = tl::expected<double, EvaluationError>;

  // using T = std::remove_cvref_t<decltype(node)>;

  ReturnType operator () (std::monostate)
  {
    return tl::unexpected(EvaluationError::empty_expression());
  }

  ReturnType operator () (const FunctionNode& node)
  {
    if (world.max_recursion_depth < current_recursion_depth)
      return tl::unexpected(EvaluationError::recursion_depth_overflow());

    auto math_obj = world.get(node.name);

    std::vector<double> evaluations;
    for (const auto& subnode : node.subnodes)
    {
      auto eval = evaluate(subnode, input_vars, world, current_recursion_depth + 1);
      if (eval) [[likely]]
        evaluations.push_back(*eval);
      else [[unlikely]]
        return eval;
    }

    return std::visit(FunctionEvaluator{.world = world,
                                        .node = node,
                                        .evaluations = evaluations,
                                        .current_recursion_depth = current_recursion_depth},
                      math_obj);
  }

  ReturnType operator () (const VariableNode& node)
  {
    auto it = input_vars.find(node.name);
    if (it != input_vars.end())
      return it->second;
    else
    {
      auto math_object = world.get(node.name);

      return std::visit(VariableEvaluator{.world = world,
                                          .node = node,
                                          .current_recursion_depth = current_recursion_depth},
                        math_object);
    }
  }

  ReturnType operator () (const NumberNode& node)
  {
    return node.value;
  }

};

/// @brief evaluates a syntax tree using a given math world
/// @param tree: tree to evaluate
/// @param input_vars: variables that are given as input to the tree, will shadow any variable in the math world
/// @param world: math world (contains functions, global constants... etc)
inline tl::expected<double, EvaluationError> evaluate(const SyntaxTree& tree,
                                                      const name_map<double>& input_vars,
                                                      const MathWorld& world,
                                                      size_t current_recursion_depth)
{
  return std::visit(NodeEvaluator{.world = world,
                                  .input_vars = input_vars,
                                  .current_recursion_depth = current_recursion_depth},
                    tree);
}

/// @brief evaluates a syntax tree using a given math world
inline tl::expected<double, EvaluationError> evaluate(const SyntaxTree& tree,
                                                      const name_map<double>& input_vars,
                                                      const MathWorld& world)
{
  return std::visit(NodeEvaluator{.world = world, .input_vars = input_vars}, tree);
}

/// @brief evaluates a syntax tree using a given math world
inline tl::expected<double, EvaluationError> evaluate(const SyntaxTree& tree, const MathWorld& world)
{
  return evaluate(tree, {}, world, 0);
}


}
