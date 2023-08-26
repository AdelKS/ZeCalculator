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

#include <zecalculator/math_objects/decl/function.h>
#include <zecalculator/parsing/parser.h>

#include <zecalculator/evaluation/ast/evaluation.h>

namespace zc {

template <parsing::Type type>
Function<type>::Function(std::vector<std::string> input_vars, std::string expr)
{
  set_input_vars(std::move(input_vars));
  set_expression(std::move(expr));
}

template <parsing::Type type>
void Function<type>::set_input_vars(std::vector<std::string> input_vars)
{
  auto it = std::ranges::find_if_not(input_vars, parsing::is_valid_name);
  if (it != input_vars.end())
    vars = tl::unexpected(InvalidInputVar{*it});
  else vars = std::move(input_vars);
}

template <parsing::Type type>
void Function<type>::set_expression(std::string expr)
{
  // do nothing if it's the same expression
  if (expression == expr)
    return;

  expression = std::move(expr);

  if (expression.empty())
  {
    if constexpr (type == parsing::AST)
      parsed_expr = std::monostate();
    else parsed_expr = {std::monostate()};
  }
  else
  {
    // workaround limitation in tl::expected when using and_then to implicitly converted-to types
    const auto tokens = parsing::tokenize(expression);
    if (tokens)
    {
      if constexpr (type == parsing::AST)
        parsed_expr = make_tree(tokens.value());
      else
      {
        auto expected_tree = make_tree(tokens.value());
        if (expected_tree)
          parsed_expr = parsing::make_RPN(*expected_tree);
        else parsed_expr = tl::unexpected(expected_tree.error());
      }
    }
    else parsed_expr = tl::unexpected(tokens.error());
  }
}

template <parsing::Type type>
std::optional<size_t> Function<type>::argument_size() const
{
  if (vars)
    return vars->size();
  else return {};
}

template <parsing::Type type>
Function<type>::operator bool () const
{
  if constexpr (type == parsing::AST)
    return bool(parsed_expr) and (not std::holds_alternative<std::monostate>(parsed_expr.value())) and bool(vars);
  else return bool(parsed_expr) and (not std::holds_alternative<std::monostate>(parsed_expr.value().front())) and bool(vars);
}

template <parsing::Type type>
std::variant<Ok, Empty, parsing::Error> Function<type>::parsing_status() const
{
  if (not parsed_expr.has_value())
    return parsed_expr.error();
  else if (std::holds_alternative<std::monostate>(parsed_expr.value()))
    return Empty();
  else return Ok();
}

template <parsing::Type type>
const tl::expected<ast::Tree, parsing::Error>& Function<type>::get_tree() const
  requires(type == parsing::AST)
{
  return parsed_expr;
}

template <parsing::Type type>
const tl::expected<rpn::RPN, parsing::Error>& Function<type>::get_rpn() const
  requires (type == parsing::RPN)
{
  return parsed_expr;
}

template <parsing::Type type>
tl::expected<double, eval::Error> Function<type>::evaluate(const std::vector<double>& args,
                                                            const MathWorld<type>& world,
                                                            size_t current_recursion_depth) const
{
  if (not bool(*this)) [[unlikely]]
    return tl::unexpected(eval::Error::invalid_function());
  else if (args.size() != vars->size()) [[unlikely]]
    return tl::unexpected(eval::Error::mismatched_fun_args());

  if constexpr (type == parsing::AST)
  {
    if (std::holds_alternative<std::monostate>(parsed_expr.value())) [[unlikely]]
      return tl::unexpected(eval::Error::empty_expression());
  }
  else
  {
    if (std::holds_alternative<std::monostate>(parsed_expr.value().front())) [[unlikely]]
      return tl::unexpected(eval::Error::empty_expression());
  }

  // make a keyword argument list out of the positional arguments
  // note: this overhead will be improved when we bind expressions to math worlds
  name_map<double> var_vals;
  for (size_t i = 0 ; i != vars->size() ; i++)
    var_vals[(*vars)[i]] = args[i];

  return zc::evaluate(*parsed_expr, var_vals, world, current_recursion_depth);
}

template <parsing::Type type>
tl::expected<double, eval::Error> Function<type>::evaluate(const std::vector<double>& args,
                                                            const MathWorld<type>& world) const
{
  // this function is user called, so the recursion depth is zero
  return evaluate(args, world, 0);
}

template <parsing::Type type>
tl::expected<double, eval::Error> Function<type>::operator()(const std::vector<double>& args,
                                                              const MathWorld<type>& world) const
{
  return evaluate(args, world, 0);
}

} // namespace zc
