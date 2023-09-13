#pragma once

/****************************************************************************
**  Copyright (c) 2023, Adel Kara Slimane <adel.ks@zegrapher.com>
**
**  This file is part of ZeCalculator's source code.
**
**  ZeCalculators is free software: you may copy, redistribute and/or modify it
**  under the terms of the GNU Affero General Public License as published by the
**  Free Software Foundation, either version 3 of the License, or (at your
**  option) any later version.
**
**  This file is distributed in the hope that it will be useful, but
**  WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include <zecalculator/evaluation/ast/decl/evaluation.h>

#include <zecalculator/evaluation/ast/impl/function.h>
#include <zecalculator/evaluation/ast/impl/variable.h>

namespace zc {
namespace eval {
namespace ast {

inline Node::ReturnType Node::operator () (std::monostate)
{
  return tl::unexpected(Error::empty_expression());
}

inline Node::ReturnType Node::operator () (const zc::ast::node::Function& node)
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

inline Node::ReturnType Node::operator () (const zc::ast::node::InputVariable& node)
{
  // node.index should never be bigger than input_vars.size()
  assert(node.index < input_vars.size());

  return input_vars[node.index];
}

inline Node::ReturnType Node::operator () (const zc::ast::node::Variable& node)
{
  auto math_object = world.get(node.name);

  return std::visit(Variable{.world = world,
                              .node = node,
                              .current_recursion_depth = current_recursion_depth},
                    math_object);

}

inline Node::ReturnType Node::operator () (const zc::ast::node::Number& node)
{
  return node.value;
}

}
}

/// @brief evaluates a syntax tree using a given math world
/// @param tree: tree to evaluate
/// @param input_vars: variables that are given as input to the tree, will shadow any variable in the math world
/// @param world: math world (contains functions, global constants... etc)
inline tl::expected<double, Error> evaluate(const ast::Tree& tree,
                                            std::span<const double> input_vars,
                                            const ast::MathWorld& world,
                                            size_t current_recursion_depth)
{
  return std::visit(eval::ast::Node{.world = world,
                                    .input_vars = input_vars,
                                    .current_recursion_depth = current_recursion_depth},
                    tree);
}

/// @brief evaluates a syntax tree using a given math world
inline tl::expected<double, Error> evaluate(const ast::Tree& tree,
                                            std::span<const double> input_vars,
                                            const ast::MathWorld& world)
{
  return std::visit(eval::ast::Node{.world = world, .input_vars = input_vars}, tree);
}

/// @brief evaluates a syntax tree using a given math world
inline tl::expected<double, Error> evaluate(const ast::Tree& tree, const ast::MathWorld& world)
{
  return evaluate(tree, std::span<const double>(), world, 0);
}


}
