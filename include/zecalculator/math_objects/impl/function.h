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

#include <zecalculator/evaluation/fast/impl/evaluation.h>
#include <zecalculator/evaluation/rpn/impl/evaluation.h>
#include <zecalculator/math_objects/decl/function.h>
#include <zecalculator/math_objects/impl/math_eq_object.h>
#include <zecalculator/parsing/data_structures/impl/uast.h>
#include <zecalculator/parsing/impl/parser.h>

#include <unordered_set>

namespace zc {

template <parsing::Type type, size_t args_num>
Function<type, args_num>::Function(MathEqObject<type> base,
                                   std::array<parsing::tokens::Text, args_num> vars)
  : MathEqObject<type>(std::move(base)),
    vars(std::move(vars)),
    direct_deps(parsing::direct_dependencies(this->rhs))
{
  rebind();
}

template <parsing::Type type, size_t args_num>
void Function<type, args_num>::rebind()
{
  if constexpr (type == parsing::Type::FAST)
    bound_rhs = parsing::bind<type>{this->m_equation, *this->mathworld}(this->rhs);
  else
    bound_rhs = parsing::bind<type>{this->m_equation, *this->mathworld}(this->rhs).transform(parsing::make_RPN);
}

template <parsing::Type type, size_t args_num>
Function<type, args_num>::operator bool () const
{
  return bool(bound_rhs);
}

template <parsing::Type type, size_t args_num>
std::optional<Error> Function<type, args_num>::error() const
{
  if (not bound_rhs)
    return bound_rhs.error();
  else return {};
}

template <parsing::Type type, size_t args_num>
const std::unordered_map<std::string, deps::ObjectType>& Function<type, args_num>::direct_dependencies() const
{
  return direct_deps;
}

template <parsing::Type type, size_t args_num>
std::unordered_map<std::string, deps::ObjectType> Function<type, args_num>::dependencies() const
{
  std::unordered_map<std::string, deps::ObjectType> deps = direct_dependencies();
  std::unordered_set<std::string> explored_deps = {this->name.substr};

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

    const auto* dyn_obj = this->mathworld->get(name);
    if (not dyn_obj or not *dyn_obj)
      continue;

    std::visit(
      [&]<class T>(const T& val) {

        if constexpr (utils::is_any_of<T, GlobalConstant<type>, GlobalVariable<type>>)
          deps.insert({val.get_name(), deps::ObjectType::VARIABLE});
        else deps.insert({val.get_name(), deps::ObjectType::FUNCTION});

        if constexpr (requires { val.direct_dependencies(); })
        {
          const auto new_deps = val.direct_dependencies();
          for (auto&& dep: new_deps)
          {
            deps.insert(dep);
            add_dep_to_explore(dep);
          }
        }
      },
      **dyn_obj);
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

  return zc::evaluate(bound_rhs.value(), args, current_recursion_depth);
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
