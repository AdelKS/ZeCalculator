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

#include <zecalculator/evaluation/ast/impl/evaluation.h>
#include <zecalculator/evaluation/rpn/impl/evaluation.h>
#include <zecalculator/math_objects/decl/function.h>
#include <zecalculator/parsing/impl/parser.h>

namespace zc {

template <parsing::Type type>
Function<type>::Function(const MathWorld<type>& mathworld)
  requires(type == parsing::Type::AST)
  : mathworld(mathworld)
{}

template <parsing::Type type>
Function<type>::Function(const MathWorld<type>& mathworld)
  requires(type == parsing::Type::RPN)
  : parsed_expr({std::monostate()}), mathworld(mathworld)
{}

template <parsing::Type type>
Function<type>::Function(std::vector<std::string> input_vars,
                         std::string expr,
                         const MathWorld<type>& mathworld)
  : Function<type>::Function(mathworld)
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
    if constexpr (type == parsing::Type::AST)
      parsed_expr = parsing::Tree<parsing::Type::AST>(std::monostate());
    else parsed_expr = {std::monostate()};
  }
  else
  {
    // just a shortcut to have tokens -> make_tree(tokens, mathworld, vars.value)
    using namespace std::placeholders;
    auto bound_make_tree = [&](const std::vector<parsing::Token>& vec)
    {
      return parsing::make_tree<type>(vec, mathworld, vars.value());
    };

    if constexpr (type == parsing::Type::AST)
      parsed_expr = parsing::tokenize(expression).and_then(bound_make_tree);
    else
      parsed_expr
        = parsing::tokenize(expression).and_then(bound_make_tree).transform(parsing::make_RPN);
  }
}

template <parsing::Type type>
void Function<type>::set(std::vector<std::string> input_vars, std::string expr)
{
  set_input_vars(std::move(input_vars));
  set_expression(std::move(expr));
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
  if constexpr (type == parsing::Type::AST)
    return bool(parsed_expr) and (not std::holds_alternative<std::monostate>(**parsed_expr)) and bool(vars);
  else return bool(parsed_expr) and (not std::holds_alternative<std::monostate>(parsed_expr.value().front())) and bool(vars);
}

template <parsing::Type type>
std::variant<Ok, Empty, Error> Function<type>::parsing_status() const
{
  if constexpr (type == parsing::Type::AST)
  {
    if (not parsed_expr.has_value())
      return parsed_expr.error();
    else if (std::holds_alternative<std::monostate>(*(parsed_expr.value())))
      return Empty();
    else return Ok();
  }
  else
  {
    if (not parsed_expr.has_value())
      return parsed_expr.error();
    else if (std::holds_alternative<std::monostate>(parsed_expr.value().front()))
      return Empty();
    else return Ok();
  }

}

template <parsing::Type type>
const tl::expected<parsing::Parsing<type>, Error>& Function<type>::get_parsing() const
{
  return parsed_expr;
}

template <parsing::Type type>
tl::expected<double, Error> Function<type>::evaluate(std::span<const double> args,
                                                     size_t current_recursion_depth) const
{
  if (mathworld.max_recursion_depth < current_recursion_depth) [[unlikely]]
    return tl::unexpected(Error::recursion_depth_overflow());
  else if (not bool(*this)) [[unlikely]]
    return tl::unexpected(Error::invalid_function());
  else if (args.size() != vars->size()) [[unlikely]]
    return tl::unexpected(Error::mismatched_fun_args());

  if constexpr (type == parsing::Type::AST)
  {
    if (std::holds_alternative<std::monostate>(*parsed_expr.value())) [[unlikely]]
      return tl::unexpected(Error::empty_expression());
  }
  else
  {
    if (std::holds_alternative<std::monostate>(parsed_expr.value().front())) [[unlikely]]
      return tl::unexpected(Error::empty_expression());
  }

  return zc::evaluate(*parsed_expr, args, current_recursion_depth);
}

template <parsing::Type type>
tl::expected<double, Error> Function<type>::evaluate(const std::vector<double>& args) const
{
  // this function is user called, so the recursion depth is zero
  return evaluate(args, 0);
}

template <parsing::Type type>
tl::expected<double, Error> Function<type>::evaluate(std::span<const double> args) const
{
  // this function is user called, so the recursion depth is zero
  return evaluate(args, 0);
}

template <parsing::Type type>
tl::expected<double, Error> Function<type>::operator()(const std::vector<double>& args) const
{
  return evaluate(args, 0);
}

template <parsing::Type type>
tl::expected<double, Error> Function<type>::operator()(std::span<const double> args) const
{
  return evaluate(args, 0);
}

} // namespace zc
