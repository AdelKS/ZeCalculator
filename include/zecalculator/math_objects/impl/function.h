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
#include <zecalculator/math_objects/impl/math_object.h>
#include <zecalculator/parsing/data_structures/impl/uast.h>
#include <zecalculator/parsing/impl/parser.h>

#include <unordered_set>

namespace zc {

template <parsing::Type type, size_t args_num>
Function<type, args_num>::Function(const MathWorld<type>* mathworld)
  : MathObject<type>(mathworld), tokenized_expr(tl::unexpected(Error::empty_expression())),
    parsed_expr(tl::unexpected(Error::empty_expression()))
{}

template <parsing::Type type, size_t args_num>
void Function<type, args_num>::set_input_vars(Vars<args_num> input_vars)
  requires (args_num > 0 )
{
  auto it = std::ranges::find_if_not(input_vars, parsing::is_valid_name);
  if (it != input_vars.end())
    this->vars = tl::unexpected(
      Error::wrong_format(parsing::tokens::Text(*it, SubstrInfo{.size = (*it).size()})));
  else
  {
    this->vars = std::move(input_vars);
    parse();
  }
}

template <parsing::Type type, size_t args_num>
void Function<type, args_num>::set_input_var(std::string input_var)
  requires (args_num == 1)
{
  if (parsing::is_valid_name(input_var)) [[unlikely]]
    this->vars = tl::unexpected(
      Error::wrong_format(parsing::tokens::Text(input_var, SubstrInfo{.size = input_var.size()})));
  else
  {
    this->vars = std::array{std::move(input_var)};
    parse();
  }
}

template <parsing::Type type, size_t args_num>
void Function<type, args_num>::set_expression(std::string expr)
{
  // do nothing if it's the same expression
  if (expression == expr)
    return;

  expression = std::move(expr);

  if (expression.empty())
    tokenized_expr = tl::unexpected(Error::empty_expression());
  else tokenized_expr = parsing::tokenize(expression);

  parse();
}

template <parsing::Type type, size_t args_num>
void Function<type, args_num>::parse()
{
  if constexpr (args_num > 0)
  {
    if (not this->vars)
    {
      parsed_expr = tl::unexpected(Error::invalid_function());
      return;
    }
  }

  auto make_uast = [&](std::span<const parsing::Token> tokens) -> tl::expected<parsing::UAST, Error>
  {
    if constexpr (args_num == 0)
      return parsing::make_uast(tokens, {});
    else return parsing::make_uast(tokens, this->vars.value());
  };

  using namespace std::placeholders;

  if constexpr (type == parsing::Type::AST)
    parsed_expr = tokenized_expr.and_then(make_uast).and_then(parsing::bind<type>{*this->mathworld});
  else
    parsed_expr
      = tokenized_expr.and_then(make_uast).and_then(parsing::bind<type>{*this->mathworld}).transform(parsing::make_RPN);
}

template <parsing::Type type, size_t args_num>
void Function<type, args_num>::set(Vars<args_num> input_vars, std::string expr)
  requires (args_num >= 1)
{
  set_input_vars(std::move(input_vars));
  set_expression(std::move(expr));
}

template <parsing::Type type, size_t args_num>
void Function<type, args_num>::set(std::string expr)
  requires (args_num == 0)
{
  set_expression(std::move(expr));
}

template <parsing::Type type, size_t args_num>
Function<type, args_num>::operator bool () const
{
  if constexpr (args_num == 0)
    return bool(parsed_expr);
  else return bool(parsed_expr) and bool(this->vars);
}

template <parsing::Type type, size_t args_num>
std::optional<Error> Function<type, args_num>::error() const
{
  if constexpr (args_num > 0)
  {
    if (not this->vars)
      return this->vars.error();
  }

  if (not parsed_expr)
    return parsed_expr.error();
  else
    return {};
}

template <parsing::Type type, size_t args_num>
const tl::expected<parsing::Parsing<type>, Error>& Function<type, args_num>::get_parsing() const
{
  return parsed_expr;
}

template <parsing::Type type, size_t args_num>
std::unordered_map<std::string, deps::ObjectType> Function<type, args_num>::direct_dependencies() const
{
  if (not tokenized_expr.has_value())
    return {};

  std::unordered_map<std::string, deps::ObjectType> deps;

  for (const parsing::Token& tok: tokenized_expr.value())
    std::visit(
      utils::overloaded{
        [&](const parsing::tokens::Function& f) {
          deps.insert({f.name, deps::ObjectType::FUNCTION});
        },
        [&](const parsing::tokens::Variable& v) {
          if (not vars.has_value() or std::ranges::count(*vars, v.name) == 0)
            deps.insert({v.name, deps::ObjectType::VARIABLE});
        },
        [](auto&&){ /* no op */ },
      }, tok);

  return deps;
}

template <parsing::Type type, size_t args_num>
std::unordered_map<std::string, deps::ObjectType> Function<type, args_num>::dependencies() const
{
  std::unordered_map<std::string, deps::ObjectType> deps = direct_dependencies();
  std::unordered_set<std::string> explored_deps = {this->name};

  std::unordered_set<std::string> to_explore;

  auto add_dep_to_explore = [&](const std::pair<std::string,  deps::ObjectType>& dep) {
    if (dep.second == deps::ObjectType::FUNCTION and not explored_deps.contains(dep.first))
      to_explore.insert(dep.first);
  };

  for (auto&& dep: deps)
    add_dep_to_explore(dep);

  while (not to_explore.empty())
  {
    // pop a node out and only keep the name
    std::string name = to_explore.extract(to_explore.begin()).value();
    explored_deps.insert(name);

    std::visit(
      [&]<class T>(T&& val) {
        if constexpr (requires { val->get_name(); })
        {
          using Type = std::remove_pointer_t<std::remove_cvref_t<T>>;
          if constexpr (utils::is_any_of<Type, GlobalConstant<type>, GlobalVariable<type>>)
            deps.insert({val->get_name(), deps::ObjectType::VARIABLE});
          else deps.insert({val->get_name(), deps::ObjectType::FUNCTION});
        }
        if constexpr (requires { val->direct_dependencies(); })
        {
          const auto new_deps = val->direct_dependencies();
          for (auto&& dep: new_deps)
          {
            deps.insert(dep);
            add_dep_to_explore(dep);
          }
        }
      },
      this->mathworld->get(name));
  }
  return deps;
}

template <parsing::Type type, size_t args_num>
tl::expected<double, Error> Function<type, args_num>::evaluate(
  std::span<const double, args_num> args, size_t current_recursion_depth) const
{
  if (this->mathworld->max_recursion_depth < current_recursion_depth) [[unlikely]]
    return tl::unexpected(Error::recursion_depth_overflow());
  else if (not bool(*this)) [[unlikely]]
    return tl::unexpected(*error());

  return zc::evaluate(*parsed_expr, args, current_recursion_depth);
}

template <parsing::Type type, size_t args_num>
tl::expected<double, Error> Function<type, args_num>::evaluate(const std::array<double, args_num>& args) const
  requires (args_num >= 1)
{
  // this function is user called, so the recursion depth is zero
  return evaluate(args, 0);
}

template <parsing::Type type, size_t args_num>
tl::expected<double, Error> Function<type, args_num>::evaluate(std::span<const double, args_num> args) const
  requires (args_num >= 1)
{
  // this function is user called, so the recursion depth is zero
  return evaluate(args, 0);
}

template <parsing::Type type, size_t args_num>
tl::expected<double, Error> Function<type, args_num>::operator()(const std::array<double, args_num>& args) const
  requires (args_num >= 1)
{
  return evaluate(args, 0);
}

template <parsing::Type type, size_t args_num>
tl::expected<double, Error> Function<type, args_num>::operator()(std::span<const double, args_num> args) const
  requires (args_num >= 1)
{
  return evaluate(args, 0);
}

template <parsing::Type type, size_t args_num>
tl::expected<double, Error> Function<type, args_num>::evaluate() const
  requires (args_num == 0)
{
  return evaluate({}, 0);
}

template <parsing::Type type, size_t args_num>
tl::expected<double, Error> Function<type, args_num>::operator()() const
  requires (args_num == 0)
{
  return evaluate({}, 0);
}

} // namespace zc
