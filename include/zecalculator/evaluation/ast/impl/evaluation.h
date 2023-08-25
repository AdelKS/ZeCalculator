#pragma once

#include <zecalculator/evaluation/ast/decl/evaluation.h>

#include <zecalculator/evaluation/ast/impl/function.h>
#include <zecalculator/evaluation/ast/impl/variable.h>

namespace zc {
namespace eval {


inline Node::ReturnType Node::operator () (std::monostate)
{
  return tl::unexpected(Error::empty_expression());
}

inline Node::ReturnType Node::operator () (const ast::node::Function& node)
{
  if (world.max_recursion_depth < current_recursion_depth)
    return tl::unexpected(Error::recursion_depth_overflow());

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

  return std::visit(Function{.world = world,
                              .node = node,
                              .evaluations = evaluations,
                              .current_recursion_depth = current_recursion_depth},
                    math_obj);
}

inline Node::ReturnType Node::operator () (const ast::node::Variable& node)
{
  auto it = input_vars.find(node.name);
  if (it != input_vars.end())
    return it->second;
  else
  {
    auto math_object = world.get(node.name);

    return std::visit(Variable{.world = world,
                                .node = node,
                                .current_recursion_depth = current_recursion_depth},
                      math_object);
  }
}

inline Node::ReturnType Node::operator () (const ast::node::Number& node)
{
  return node.value;
}

}

/// @brief evaluates a syntax tree using a given math world
/// @param tree: tree to evaluate
/// @param input_vars: variables that are given as input to the tree, will shadow any variable in the math world
/// @param world: math world (contains functions, global constants... etc)
inline tl::expected<double, eval::Error> evaluate(const ast::Tree& tree,
                                                  const name_map<double>& input_vars,
                                                  const MathWorld& world,
                                                  size_t current_recursion_depth)
{
  return std::visit(eval::Node{.world = world,
                               .input_vars = input_vars,
                               .current_recursion_depth = current_recursion_depth},
                    tree);
}

/// @brief evaluates a syntax tree using a given math world
inline tl::expected<double, eval::Error> evaluate(const ast::Tree& tree,
                                                  const name_map<double>& input_vars,
                                                  const MathWorld& world)
{
  return std::visit(eval::Node{.world = world, .input_vars = input_vars}, tree);
}

/// @brief evaluates a syntax tree using a given math world
inline tl::expected<double, eval::Error> evaluate(const ast::Tree& tree, const MathWorld& world)
{
  return evaluate(tree, {}, world, 0);
}


}
