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

#include <zecalculator/error.h>
#include <zecalculator/evaluation/decl/cache.h>
#include <zecalculator/mathworld/decl/mathworld.h>
#include <zecalculator/parsing/data_structures/decl/fast.h>
#include <zecalculator/utils/name_map.h>

namespace zc {
namespace eval {

/// @brief maximum recursion depth to reach before returning an error
inline size_t max_recursion_depth = 20;

template <parsing::Type type>
struct Evaluator
{
  using ValuesContainer =
    std::conditional_t<type == parsing::Type::FAST,
                       std::span<const double>,
                       std::vector<double>>;

  std::span<const double> input_vars;
  ValuesContainer subnodes = {};
  const size_t current_recursion_depth = 0;
  Cache* cache = nullptr;

  /// @brief only used in the RPN case, too lazy to remove it otherwise
  Error error = {};

  using RetType = std::conditional_t<type == parsing::Type::FAST, tl::expected<double, Error>, bool>;

  template <class Op>
  auto handle_binary_operator(Op&&) -> RetType;

  template <class Op>
  auto handle_unary_operator(Op&&) -> RetType;

  void update_stack(double to_push, size_t values_consumed);

  auto operator () (zc::parsing::shared::node::Add) -> RetType;
  auto operator () (zc::parsing::shared::node::Subtract) -> RetType;
  auto operator () (zc::parsing::shared::node::Multiply) -> RetType;
  auto operator () (zc::parsing::shared::node::Divide) -> RetType;
  auto operator () (zc::parsing::shared::node::Power) -> RetType;
  auto operator () (zc::parsing::shared::node::UnaryMinus) -> RetType;

  auto operator () (const zc::parsing::LinkedFunc<type>*) -> RetType;

  template <class T>
    requires utils::is_any_of<T, zc::parsing::LinkedSeq<type>, zc::parsing::LinkedData<type>>
  auto operator () (const T*) -> RetType;

  auto operator () (const zc::parsing::shared::node::InputVariable&) -> RetType;

  auto operator () (const zc::parsing::shared::node::Number&) -> RetType;

  template <size_t args_num>
  auto operator () (zc::CppFunction<args_num>) -> RetType;

  auto operator () (const double*) -> RetType;

};

} // namespace eval

/// ================= FAST

/// @brief evaluates a syntax tree using a given math world
/// @param tree: tree to evaluate
/// @param input_vars: variables that are given as input to the tree, will shadow any variable in the math world
/// @param world: math world (contains functions, global constants... etc)
tl::expected<double, Error> evaluate(const parsing::FAST<parsing::Type::FAST>& tree,
                                     std::span<const double> input_vars,
                                     size_t current_recursion_depth,
                                     eval::Cache* cache = nullptr);

/// @brief evaluates a syntax tree using a given math world
tl::expected<double, Error> evaluate(const parsing::FAST<parsing::Type::FAST>& tree,
                                     std::span<const double> input_vars,
                                     eval::Cache* cache = nullptr);

/// @brief evaluates a syntax tree using a given math world
tl::expected<double, Error> evaluate(const parsing::FAST<parsing::Type::FAST>& tree,
                                     eval::Cache* cache = nullptr);

/// ================= RPN

/// @brief evaluates a syntax tree using a given math world
/// @param tree: tree to evaluate
/// @param input_vars: variables that are given as input to the tree, will shadow any variable in the math world
/// @param world: math world (contains functions, global constants... etc)
tl::expected<double, Error> evaluate(const parsing::RPN& rpn,
                                     std::span<const double> input_vars,
                                     size_t current_recursion_depth,
                                     eval::Cache* cache = nullptr);

/// @brief evaluates a syntax tree using a given math world
tl::expected<double, Error> evaluate(const parsing::RPN& rpn,
                                     std::span<const double> input_vars,
                                     eval::Cache* cache = nullptr);

/// @brief evaluates a syntax tree using a given math world
tl::expected<double, Error> evaluate(const parsing::RPN& rpn, eval::Cache* cache = nullptr);

// Specific to sequences and data

template <class T>
  requires utils::is_any_of<T,
                            zc::parsing::LinkedSeq<parsing::Type::RPN>,
                            zc::parsing::LinkedData<parsing::Type::RPN>,
                            zc::parsing::LinkedSeq<parsing::Type::FAST>,
                            zc::parsing::LinkedData<parsing::Type::FAST>>
tl::expected<double, Error>
  evaluate(const T& u, double index, size_t current_recursion_depth, eval::Cache* cache = nullptr);

template <class T>
  requires utils::is_any_of<T,
                            zc::parsing::LinkedSeq<parsing::Type::RPN>,
                            zc::parsing::LinkedData<parsing::Type::RPN>,
                            zc::parsing::LinkedSeq<parsing::Type::FAST>,
                            zc::parsing::LinkedData<parsing::Type::FAST>>
tl::expected<double, Error>
  evaluate(const T& u, double index, eval::Cache* cache = nullptr);

} // namespace zc
