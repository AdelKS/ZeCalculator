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
#include <zecalculator/parsing/data_structures/impl/ast.h>

namespace zc {
namespace eval {
namespace ast {

template <size_t input_size>
template <size_t args_num>
inline Evaluator<input_size>::ReturnType Evaluator<input_size>::operator()(
  const zc::parsing::ast::node::Function<zc::parsing::Type::AST, args_num>& node)
{
  if (node.f->mathworld->max_recursion_depth < current_recursion_depth)
    return tl::unexpected(Error::recursion_depth_overflow());

  std::array<double, args_num> evaluations;
  size_t i = 0;
  for (const auto& operand : node.operands)
  {
    auto eval = evaluate(operand, input_vars, current_recursion_depth + 1);
    if (eval) [[likely]]
      evaluations[i] = *eval;
    else [[unlikely]]
      return eval;
    i++;
  }

  return node.f->evaluate(evaluations, current_recursion_depth + 1);
}

template <size_t input_size>
template <char op, size_t args_num>
inline Evaluator<input_size>::ReturnType Evaluator<input_size>::operator()(
  const zc::parsing::ast::node::Operator<zc::parsing::Type::AST, op, args_num>& node)
{
  std::array<double, args_num> evaluations;
  size_t i = 0;
  for (const auto& operand : node.operands)
  {
    auto eval = evaluate(operand, input_vars, current_recursion_depth + 1);
    if (eval) [[likely]]
      evaluations[i] = *eval;
    else [[unlikely]]
      return eval;
    i++;
  }

  if constexpr (args_num == 2)
  {
    if constexpr (op == '+')
      return evaluations[0] + evaluations[1];
    else if constexpr (op == '-')
      return evaluations[0] - evaluations[1];
    else if constexpr (op == '*')
      return evaluations[0] * evaluations[1];
    else if constexpr (op == '/')
      return evaluations[0] / evaluations[1];
    else if constexpr (op == '^')
      return std::pow(evaluations[0], evaluations[1]);
    else if constexpr (op == '=')
      return evaluations[0] == evaluations[1];
    else static_assert(utils::dependent_false_num_v<op>, "case not handled");
  }
  else static_assert(utils::dependent_false_num_v<args_num>, "case not handled");
}

template <size_t input_size>
inline Evaluator<input_size>::ReturnType Evaluator<input_size>::operator()(
  const zc::parsing::ast::node::Sequence<zc::parsing::Type::AST>& node)
{
  if (node.u->mathworld->max_recursion_depth < current_recursion_depth)
    return tl::unexpected(Error::recursion_depth_overflow());

  auto eval = evaluate(node.operand, input_vars, current_recursion_depth + 1);
  if (eval) [[likely]]
    return node.u->evaluate(*eval, current_recursion_depth + 1);
  else [[unlikely]]
    return eval;
}

template <size_t input_size>
template <size_t args_num>
inline Evaluator<input_size>::ReturnType Evaluator<input_size>::operator()(
  const zc::parsing::ast::node::CppFunction<zc::parsing::Type::AST, args_num>& node)
{
  std::array<double, args_num> evals;
  for (size_t i = 0 ; i != args_num ; i++)
  {
    auto res = evaluate(node.operands[i], input_vars, current_recursion_depth + 1);
    if (not res) [[unlikely]]
      return res;
    else evals[i] = *res;
  }

  auto unpack_compute = [&]<size_t... i>(std::integer_sequence<size_t, i...>)
  {
    return (*node.f)(evals[i]...);
  };
  return unpack_compute(std::make_index_sequence<args_num>());

}

template <size_t input_size>
inline Evaluator<input_size>::ReturnType Evaluator<input_size>::operator () (
  const zc::parsing::shared::node::GlobalConstant<zc::parsing::Type::AST>& node)
{
  return node.constant->value;
}

template <size_t input_size>
inline Evaluator<input_size>::ReturnType
  Evaluator<input_size>::operator()(const zc::parsing::shared::node::InputVariable& node)
{
  // node.index should never be bigger than input_vars.size()
  assert(node.index < input_vars.size());

  return input_vars[node.index];
}

template <size_t input_size>
inline Evaluator<input_size>::ReturnType
  Evaluator<input_size>::operator()(const zc::parsing::shared::node::Number& node)
{
  return node.value;
}

}
}

/// @brief evaluates a syntax tree using a given math world
/// @param tree: tree to evaluate
/// @param input_vars: variables that are given as input to the tree, will shadow any variable in the math world
/// @param world: math world (contains functions, global constants... etc)
template <size_t input_size>
inline tl::expected<double, Error> evaluate(const parsing::AST<parsing::Type::AST>& tree,
                                            std::span<const double, input_size> input_vars,
                                            size_t current_recursion_depth)
{
  return std::visit(eval::ast::Evaluator<input_size>{.input_vars = input_vars,
                                                     .current_recursion_depth
                                                     = current_recursion_depth},
                    *tree);
}

/// @brief evaluates a syntax tree using a given math world
template <size_t input_size>
inline tl::expected<double, Error> evaluate(const parsing::AST<parsing::Type::AST>& tree,
                                            std::span<const double, input_size> input_vars)
{
  return evaluate(tree, input_vars, 0);
}

/// @brief evaluates a syntax tree using a given math world
inline tl::expected<double, Error> evaluate(const parsing::AST<parsing::Type::AST>& tree)
{
  return evaluate(tree, std::span<const double>(), 0);
}


}
