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

#include "zecalculator/evaluation/error.h"
#include <variant>
#include <zecalculator/evaluation/rpn/decl/evaluation.h>

#include <zecalculator/evaluation/rpn/impl/function.h>
#include <zecalculator/evaluation/rpn/impl/variable.h>

namespace zc {
namespace eval {
namespace rpn {


inline void RPN::operator () (std::monostate)
{
  expected_eval_stack = tl::unexpected(Error::empty_expression());
}

inline void RPN::operator () (const parsing::tokens::Function& func_token)
{
  if (world.max_recursion_depth < current_recursion_depth) [[unlikely]]
  {
    expected_eval_stack = tl::unexpected(Error::recursion_depth_overflow());
    return;
  }

  auto math_obj = world.get(func_token.name);

  std::visit(Function{.world = world,
                      .func_token = func_token,
                      .expected_eval_stack = expected_eval_stack,
                      .current_recursion_depth = current_recursion_depth},
             math_obj);
}

inline void RPN::operator () (const parsing::tokens::Variable& var_token)
{
  auto it = input_vars.find(var_token.name);
  if (it != input_vars.end())
    expected_eval_stack->push_back(it->second);
  else
  {
    auto math_object = world.get(var_token.name);

    std::visit(Variable{.world = world,
                        .var_token = var_token,
                        .expected_eval_stack = expected_eval_stack,
                        .current_recursion_depth = current_recursion_depth},
               math_object);
  }
}

inline void RPN::operator () (const parsing::tokens::Number& node)
{
  expected_eval_stack->push_back(node.value);
}

}
}

/// @brief evaluates a syntax expr using a given math world
/// @param expr: expr to evaluate
/// @param input_vars: variables that are given as input to the expr, will shadow any variable in the math world
/// @param world: math world (contains functions, global constants... etc)
inline tl::expected<double, eval::Error> evaluate(const rpn::RPN& expr,
                                                  const name_map<double>& input_vars,
                                                  const rpn::MathWorld& world,
                                                  size_t current_recursion_depth)
{
  eval::rpn::RPN stateful_evaluator{.world = world,
                                    .input_vars = input_vars,
                                    .current_recursion_depth = current_recursion_depth};

  for (const rpn::Token& tok: expr)
  {
    std::visit(stateful_evaluator, tok);
    if (not stateful_evaluator.expected_eval_stack)
      break;
  }

  if (stateful_evaluator.expected_eval_stack)
  {
    if (stateful_evaluator.expected_eval_stack->size() == 1)
      return stateful_evaluator.expected_eval_stack->front();
    else return tl::unexpected(eval::Error::unkown());
  }
  else return tl::unexpected(stateful_evaluator.expected_eval_stack.error());
}

/// @brief evaluates a syntax expr using a given math world
inline tl::expected<double, eval::Error> evaluate(const rpn::RPN& expr,
                                                  const name_map<double>& input_vars,
                                                  const rpn::MathWorld& world)
{
  return evaluate(expr, input_vars, world, 0);
}

/// @brief evaluates a syntax expr using a given math world
inline tl::expected<double, eval::Error> evaluate(const rpn::RPN& expr, const rpn::MathWorld& world)
{
  return evaluate(expr, {}, world, 0);
}


}
