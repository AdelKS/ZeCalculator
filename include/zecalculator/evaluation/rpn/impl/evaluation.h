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

#include <variant>
#include <zecalculator/error.h>
#include <zecalculator/evaluation/rpn/decl/evaluation.h>
#include <zecalculator/math_objects/impl/function.h>
#include <zecalculator/parsing/data_structures/impl/rpn.h>

namespace zc {
namespace eval {
namespace rpn {

template <size_t input_size>
inline void Evaluator<input_size>::operator () (const zc::parsing::shared::node::Number& val)
{
  expected_eval_stack->push_back(val.value);
}

template <size_t input_size>
template <size_t args_num>
inline void Evaluator<input_size>::operator()(const zc::parsing::rpn::node::Function<args_num>& node)
{
  if (not bool(node.f)) [[unlikely]]
    expected_eval_stack = tl::unexpected(Error::object_in_invalid_state(node, node.f->equation()));
  else
  {
    const auto evaluations = std::span<const double, args_num>(expected_eval_stack->end() - args_num,
                                                               args_num);

    auto expected_res = node.f->evaluate(evaluations, current_recursion_depth + 1);

    // when done, remove args_num from the back of the evaluation stack
    // note: the resize needs to be done before pushing in the new result
    expected_eval_stack->resize(expected_eval_stack->size() - args_num);

    if (expected_res)
      // push the result to the evaluation stack if correct
      expected_eval_stack->push_back(*expected_res);

    // save error in the expected otherwise
    else expected_eval_stack = tl::unexpected(expected_res.error());
  }
}

template <size_t input_size>
template <size_t args_num>
inline void
  Evaluator<input_size>::operator()(const zc::parsing::rpn::node::CppFunction<args_num>& node)
{
  // points on the before last value on the stack
  const auto it = expected_eval_stack->end() - args_num;

  auto compute_overwrite_val = [&]<size_t... i>(std::integer_sequence<size_t, i...>)
  {
    // since the function pops two elements, then pushes back one
    // we can overwrite directly the value that will get replaced
    *it = (*node.f)(*(it+i)...);
  };
  compute_overwrite_val(std::make_index_sequence<args_num>());

  // remove args_num-1 values from the stack,
  // why the minus one: one value got overwritten with the computation result, as an optim
  if constexpr (args_num >= 2)
    expected_eval_stack->resize(expected_eval_stack->size() - (args_num - 1));
}

template <size_t input_size>
template <char op, size_t args_num>
inline void Evaluator<input_size>::operator()(const zc::parsing::rpn::node::Operator<op, args_num>&)
{
  // points on the before last value on the stack
  const auto it = expected_eval_stack->end() - args_num;

  if constexpr (args_num == 2)
  {
    if constexpr (op == '+')
      *it = *(it) + *(it+1);
    else if constexpr (op == '-')
      *it = *(it) - *(it+1);
    else if constexpr (op == '*')
      *it = *(it) * *(it+1);
    else if constexpr (op == '/')
      *it = *(it) / *(it+1);
    else if constexpr (op == '^')
      *it = std::pow(*(it), *(it+1));
    else if constexpr (op == '=')
      *it = (*(it) == *(it+1));
    else static_assert(utils::dependent_false_num_v<op>, "case not handled");
  }
  else static_assert(utils::dependent_false_num_v<args_num>, "case not handled");

  // remove args_num-1 values from the stack,
  // why the minus one: one value got overwritten with the computation result, as an optim
  if constexpr (args_num >= 2)
    expected_eval_stack->resize(expected_eval_stack->size() - (args_num - 1));
}

template <size_t input_size>
inline void Evaluator<input_size>::operator()(const zc::parsing::rpn::node::Sequence& node)
{
  //              std::cout << "Evaluating zc function: " << node.name << std::endl;
  if (not bool(node.u))
    expected_eval_stack = tl::unexpected(Error::object_in_invalid_state(node, node.u->equation()));
  else
  {
    // sequence handles only one argument
    double& back_val = expected_eval_stack->back();

    auto expected_res = node.u->evaluate(back_val, current_recursion_depth + 1);

    if (expected_res)
      // overwrite the top of the stack on computation success
      back_val = *expected_res;

    // save error in the expected otherwise
    else expected_eval_stack = tl::unexpected(expected_res.error());
  }
}

template <size_t input_size>
inline void Evaluator<input_size>::operator () (const zc::parsing::shared::node::InputVariable& in_var)
{
  // node.index should never be bigger than input_vars.size()
  assert(in_var.index < input_vars.size());

  expected_eval_stack->push_back(input_vars[in_var.index]);
}

template <size_t input_size>
inline void Evaluator<input_size>::operator()(const zc::parsing::rpn::node::GlobalConstant& node)
{
  expected_eval_stack->push_back(node.constant->value());
}

}
}

/// @brief evaluates a syntax expr using a given math world
/// @param expr: expr to evaluate
/// @param input_vars: variables that are given as input to the expr, will shadow any variable in the math world
/// @param world: math world (contains functions, global constants... etc)
template <size_t input_size>
inline tl::expected<double, Error> evaluate(const parsing::RPN& expr,
                                            std::span<const double, input_size> input_vars,
                                            size_t current_recursion_depth)
{
  eval::rpn::Evaluator<input_size> stateful_evaluator{.input_vars = input_vars,
                                                      .current_recursion_depth
                                                      = current_recursion_depth};

  for (const parsing::rpn::node::Node& tok: expr)
  {
    std::visit(stateful_evaluator, tok);
    if (not stateful_evaluator.expected_eval_stack)
      break;
  }

  if (stateful_evaluator.expected_eval_stack)
  {
    if (stateful_evaluator.expected_eval_stack->size() == 1)
      return stateful_evaluator.expected_eval_stack->front();
    else return tl::unexpected(Error::unkown());
  }
  else return tl::unexpected(stateful_evaluator.expected_eval_stack.error());
}

/// @brief evaluates a syntax expr using a given math world
inline tl::expected<double, Error> evaluate(const parsing::RPN& expr,
                                            std::span<const double> input_vars)
{
  return evaluate(expr, input_vars, 0);
}

/// @brief evaluates a syntax expr using a given math world
inline tl::expected<double, Error> evaluate(const parsing::RPN& expr)
{
  return evaluate(expr, std::span<const double>(), 0);
}


}
