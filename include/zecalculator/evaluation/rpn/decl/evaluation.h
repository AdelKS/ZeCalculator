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

#include "zecalculator/external/expected.h"
#include "zecalculator/parsing/data_structures/decl/rpn.h"
#include <zecalculator/error.h>
#include <zecalculator/math_objects/cpp_function.h>
#include <zecalculator/math_objects/aliases.h>
#include <zecalculator/math_objects/decl/function.h>
#include <zecalculator/math_objects/global_constant.h>
#include <zecalculator/mathworld/decl/mathworld.h>
#include <zecalculator/utils/name_map.h>

namespace zc {

namespace eval {
namespace rpn {

template <size_t input_size>
struct Evaluator
{
  const std::span<const double, input_size> input_vars;
  tl::expected<std::vector<double>, Error> expected_eval_stack = {};
  const size_t current_recursion_depth = 0;
  const size_t max_recursion_depth = 100;

  using ReturnType = tl::expected<double, Error>;

  void operator () (const zc::parsing::shared::node::Number&);

  void operator () (const zc::parsing::shared::node::InputVariable&);

  template <size_t args_num>
  void operator () (const zc::parsing::rpn::node::Function<args_num>&);

  template <char op, size_t args_num>
  void operator () (const zc::parsing::rpn::node::Operator<op, args_num>&);

  void operator () (const zc::parsing::rpn::node::Sequence&);

  template <size_t args_num>
  void operator () (const zc::parsing::rpn::node::CppFunction<args_num>&);

  void operator () (const zc::parsing::rpn::node::GlobalConstant&);

};

}
}

/// @brief evaluates a syntax tree using a given math world
/// @param tree: tree to evaluate
/// @param input_vars: variables that are given as input to the tree, will shadow any variable in the math world
/// @param world: math world (contains functions, global constants... etc)
template <size_t input_size>
inline tl::expected<double, Error> evaluate(const parsing::RPN& expr,
                                            std::span<const double, input_size> input_vars,
                                            size_t current_recursion_depth);

/// @brief evaluates a syntax tree using a given math world
template <size_t input_size>
inline tl::expected<double, Error> evaluate(const parsing::RPN& expr,
                                            std::span<const double, input_size> input_vars);

/// @brief evaluates a syntax tree using a given math world
inline tl::expected<double, Error> evaluate(const parsing::RPN& tree);
}
