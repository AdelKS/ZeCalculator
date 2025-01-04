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

#include <zecalculator/evaluation/decl/evaluation.h>
#include <zecalculator/evaluation/impl/cache.h>
#include <zecalculator/parsing/data_structures/impl/fast.h>

namespace zc {
namespace eval {

template <parsing::Type type>
template <class Op>
auto Evaluator<type>::handle_binary_operator(Op&& op) -> RetType
{
  assert(subnodes.size() >= 2);

  if constexpr (type == parsing::Type::FAST)
    assert(subnodes.size() == 2);

  double res = op(*(subnodes.end()-2), *(subnodes.end()-1));

  update_stack(res, 2);

  if constexpr (type == parsing::Type::FAST)
    return res;
  else return true;
}

template <parsing::Type type>
template <class Op>
auto Evaluator<type>::handle_unary_operator(Op&& op) -> RetType
{
  assert(subnodes.size() >= 1);

  if constexpr (type == parsing::Type::FAST)
    assert(subnodes.size() == 1);

  if constexpr (type == parsing::Type::FAST)
    return op(subnodes.back());
  else
  {
    subnodes.back() = op(subnodes.back());
    return true;
  }
}

template <parsing::Type type>
void Evaluator<type>::update_stack(double to_push, size_t values_consumed)
{
  if constexpr (type == parsing::Type::RPN)
  {
    subnodes.resize(subnodes.size() + 1 - values_consumed);
    subnodes.back() = to_push;
  }
}

template <parsing::Type type>
auto Evaluator<type>::operator () (zc::parsing::shared::node::Add) -> RetType
{
  return handle_binary_operator([](double a, double b){ return a + b; });
}

template <parsing::Type type>
auto Evaluator<type>::operator () (zc::parsing::shared::node::Subtract) -> RetType
{
  return handle_binary_operator([](double a, double b){ return a - b; });
}

template <parsing::Type type>
auto Evaluator<type>::operator () (zc::parsing::shared::node::Multiply) -> RetType
{
  return handle_binary_operator([](double a, double b){ return a * b; });
}

template <parsing::Type type>
auto Evaluator<type>::operator () (zc::parsing::shared::node::Divide) -> RetType
{
  return handle_binary_operator([](double a, double b){ return a / b; });
}

template <parsing::Type type>
auto Evaluator<type>::operator () (zc::parsing::shared::node::Power) -> RetType
{
  return handle_binary_operator([](double a, double b){ return std::pow(a, b); });
}

template <parsing::Type type>
auto Evaluator<type>::operator () (zc::parsing::shared::node::UnaryMinus) -> RetType
{
  return handle_unary_operator([](double a){ return -a; });
}

template <parsing::Type type>
auto Evaluator<type>::operator()(const zc::parsing::LinkedFunc<type>* f) -> RetType
{
  const size_t args_num = f->args_num;

  if constexpr (type == parsing::Type::FAST)
    assert(subnodes.size() == args_num);

  auto exp_res = zc::evaluate(f->repr,
                              {(subnodes.end() - args_num), args_num},
                              current_recursion_depth + 1,
                              cache);

  if constexpr (type == parsing::Type::FAST)
  {
    if (exp_res)
      return *exp_res;
    else return tl::unexpected(std::move(exp_res.error()));
  }
  else
  {
    if (exp_res)
    {
      update_stack(*exp_res, args_num);
      return true;
    }
    else
    {
      error = std::move(exp_res.error());
      return false;
    }
  }
}

template <parsing::Type type>
template <class T>
  requires utils::is_any_of<T, zc::parsing::LinkedSeq<type>, zc::parsing::LinkedData<type>>
auto Evaluator<type>::operator()(const T* u) -> RetType
{
  if constexpr (type == parsing::Type::FAST)
    assert(subnodes.size() == 1);

  auto exp_res = zc::evaluate(*u, subnodes.back(), current_recursion_depth+1, cache);

  if constexpr (type == parsing::Type::FAST)
  {
    if (exp_res)
      return *exp_res;
    else return tl::unexpected(std::move(exp_res.error()));
  }
  else
  {
    if (exp_res)
    {
      update_stack(*exp_res, 1);
      return true;
    }
    else
    {
      error = std::move(exp_res.error());
      return false;
    }
  }
}

template <parsing::Type type>
template <size_t args_num>
auto Evaluator<type>::operator()(zc::CppFunction<args_num> cpp_f) -> RetType
{
  if constexpr (type == parsing::Type::FAST)
    assert(subnodes.size() == args_num);

  size_t offset = 0;
  if constexpr (type == parsing::Type::RPN)
    offset = subnodes.size() - args_num;

  double res = cpp_f(std::span<const double, args_num>(subnodes.begin() + offset, args_num));

  update_stack(res, args_num);

  if constexpr (type == parsing::Type::FAST)
    return res;
  else return true;
}

template <parsing::Type type>
auto Evaluator<type>::operator()(const double* node) -> RetType
{
  double res = *node;

  if constexpr (type == parsing::Type::FAST)
    return res;
  else
  {
    subnodes.push_back(res);
    return true;
  }
}

template <parsing::Type type>
auto Evaluator<type>::operator()(const zc::parsing::shared::node::InputVariable& node) -> RetType
{
  // node.index should never be bigger than input_vars.size()
  assert(node.index < input_vars.size());

  if constexpr (type == parsing::Type::RPN)
  {
    subnodes.push_back(input_vars[node.index]);
    return true;
  }
  else return input_vars[node.index];
}

template <parsing::Type type>
auto Evaluator<type>::operator()(const zc::parsing::shared::node::Number& node) -> RetType
{
  if constexpr (type == parsing::Type::RPN)
  {
    subnodes.push_back(node.value);
    return true;
  }
  else return node.value;
}

} // namespace eval

/// =========================================== FAST

/// @brief evaluates a syntax tree using a given math world
/// @param tree: tree to evaluate
/// @param input_vars: variables that are given as input to the tree, will shadow any variable in the math world
/// @param world: math world (contains functions, global constants... etc)
inline tl::expected<double, Error> evaluate(const parsing::FAST<parsing::Type::FAST>& tree,
                                            std::span<const double> input_vars,
                                            size_t current_recursion_depth,
                                            eval::Cache* cache)
{
  if (eval::max_recursion_depth < current_recursion_depth) [[unlikely]]
    return tl::unexpected(Error::recursion_depth_overflow());

  std::vector<double> subnodes;
  subnodes.reserve(tree.subnodes.size());
  for (const auto& subnode : tree.subnodes)
  {
    auto eval = evaluate(subnode, input_vars, current_recursion_depth, cache);
    if (eval) [[likely]]
      subnodes.push_back(*eval);
    else [[unlikely]]
      return eval;
  }

  return std::visit(eval::Evaluator<parsing::Type::FAST>{.input_vars = input_vars,
                                                         .subnodes = subnodes,
                                                         .current_recursion_depth
                                                         = current_recursion_depth,
                                                         .cache = cache},
                    tree.node);
}

/// @brief evaluates a syntax tree using a given math world
inline tl::expected<double, Error> evaluate(const parsing::FAST<parsing::Type::FAST>& tree,
                                            std::span<const double> input_vars,
                                            eval::Cache* cache)
{
  return evaluate(tree, input_vars, 0, cache);
}

/// @brief evaluates a syntax tree using a given math world
inline tl::expected<double, Error> evaluate(const parsing::FAST<parsing::Type::FAST>& tree,
                                            eval::Cache* cache)
{
  return evaluate(tree, std::span<const double, 0>(), 0, cache);
}


/// =========================================== RPN

/// @brief evaluates a syntax tree using a given math world
/// @param tree: tree to evaluate
/// @param input_vars: variables that are given as input to the tree, will shadow any variable in the math world
/// @param world: math world (contains functions, global constants... etc)
inline tl::expected<double, Error> evaluate(const parsing::RPN& rpn,
                                            std::span<const double> input_vars,
                                            size_t current_recursion_depth,
                                            eval::Cache* cache)
{
  if (eval::max_recursion_depth < current_recursion_depth) [[unlikely]]
    return tl::unexpected(Error::recursion_depth_overflow());

  eval::Evaluator<parsing::Type::RPN> stateful_evaluator{.input_vars = input_vars,
                                                         .current_recursion_depth
                                                         = current_recursion_depth,
                                                         .cache = cache};

  stateful_evaluator.subnodes.reserve(rpn.size());

  for (const auto& node: rpn)
  {
    if(not std::visit(stateful_evaluator, node)) [[unlikely]]
      return tl::unexpected(std::move(stateful_evaluator.error));
  }

  assert(stateful_evaluator.subnodes.size() == 1);
  return stateful_evaluator.subnodes.front();
}

/// @brief evaluates a syntax tree using a given math world
inline tl::expected<double, Error> evaluate(const parsing::RPN& rpn,
                                            std::span<const double> input_vars,
                                            eval::Cache* cache)
{
  return evaluate(rpn, input_vars, 0, cache);
}

/// @brief evaluates a syntax tree using a given math world
inline tl::expected<double, Error> evaluate(const parsing::RPN& rpn, eval::Cache* cache)
{
  return evaluate(rpn, std::span<const double, 0>(), 0, cache);
}

template <class T>
  requires utils::is_any_of<T,
                            zc::parsing::LinkedSeq<parsing::Type::RPN>,
                            zc::parsing::LinkedData<parsing::Type::RPN>,
                            zc::parsing::LinkedSeq<parsing::Type::FAST>,
                            zc::parsing::LinkedData<parsing::Type::FAST>>
tl::expected<double, Error>
  evaluate(const T& u, double index, size_t current_recursion_depth, eval::Cache* cache)
{
  constexpr bool is_data = utils::is_any_of<T,
                                            zc::parsing::LinkedData<parsing::Type::RPN>,
                                            zc::parsing::LinkedData<parsing::Type::FAST>>;

  double rounded_index = std::round(index);

  tl::expected<double, zc::Error> exp_res = tl::unexpected(zc::Error::unkown());

  auto get_cached_value = [&] () -> std::optional<double> {
    if (cache)
      if (auto obj_cache_it = cache->find(u.slot); obj_cache_it != cache->end())
      {
        auto& obj_cache = obj_cache_it->second.get_cache();
        if (auto value_it = obj_cache.find(rounded_index); value_it != obj_cache.end())
          return value_it->second;
      }
    return {};
  };

  if (rounded_index < 0 or u.repr.empty() or (is_data and rounded_index >= u.repr.size())) [[unlikely]]
    exp_res = std::nan("");

  else if (auto opt_val = get_cached_value(); bool(opt_val))
    exp_res = *opt_val;

  else
  {
    size_t unsigned_index = rounded_index;

    if constexpr (is_data)
    {
      assert(unsigned_index < u.repr.size());
      const auto& exp_parsing = u.repr[unsigned_index];

      if (exp_parsing)
        exp_res = zc::evaluate(*exp_parsing, std::array{rounded_index}, current_recursion_depth, cache);
      else exp_res = tl::unexpected(exp_parsing.error());
    }
    else
    {
      const auto& parsing = unsigned_index < u.repr.size() ? u.repr[unsigned_index] : u.repr.back();

      exp_res = zc::evaluate(parsing, std::array{rounded_index}, current_recursion_depth, cache);
    }

    if (exp_res and cache)
      (*cache)[u.slot].insert(rounded_index, *exp_res);
  }

  return exp_res;
}

template <class T>
  requires utils::is_any_of<T,
                            zc::parsing::LinkedSeq<parsing::Type::RPN>,
                            zc::parsing::LinkedData<parsing::Type::RPN>,
                            zc::parsing::LinkedSeq<parsing::Type::FAST>,
                            zc::parsing::LinkedData<parsing::Type::FAST>>
tl::expected<double, Error>
  evaluate(const T& u, double index, eval::Cache* cache)
{
  return evaluate(u, index, 0, cache);
}

}
