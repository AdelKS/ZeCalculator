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

#include <zecalculator/mathworld/decl/mathworld.h>
#include <zecalculator/parsing/parser.h>

namespace zc {

template <parsing::Type type>
MathWorld<type>::MathWorld()
{
  for (auto&& [name, f_ptr]: builtin_unary_functions)
    add<CppUnaryFunction<type>>(name, f_ptr);

  for (auto&& [name, cst]: builtin_global_constants)
    add<GlobalConstant<type>>(name, cst);
}

template <parsing::Type type>
MathWorld<type>::ConstDynMathObject MathWorld<type>::get(std::string_view name) const
{
  auto it = inventory.find(name);
  return it != inventory.end() ? to_const(it->second) : ConstDynMathObject();
}

template <parsing::Type type>
MathWorld<type>::DynMathObject MathWorld<type>::get(std::string_view name)
{
  auto it = inventory.find(name);
  return it != inventory.end() ? it->second : DynMathObject();
}

template <parsing::Type type>
template <class ObjectType>
  requires tuple_contains_v<MathObjects<type>, ObjectType>
ObjectType* MathWorld<type>::get(std::string_view name)
{
  DynMathObject dyn_obj = get(name);
  if (std::holds_alternative<ObjectType*>(dyn_obj))
    return std::get<ObjectType*>(dyn_obj);
  else return nullptr;
}

template <parsing::Type type>
template <class ObjectType>
  requires tuple_contains_v<MathObjects<type>, ObjectType>
const ObjectType* MathWorld<type>::get(std::string_view name) const
{
  ConstDynMathObject dyn_obj = get(name);
  if (std::holds_alternative<const ObjectType*>(dyn_obj))
    return std::get<const ObjectType*>(dyn_obj);
  else return nullptr;
}


template <parsing::Type type>
template <class ObjectType>
  requires tuple_contains_v<MathObjects<type>, ObjectType>
ObjectType& MathWorld<type>::get(size_t id)
{
  return std::get<SlottedDeque<ObjectType>>(math_objects).at(id);
}


template <parsing::Type type>
template <class ObjectType>
  requires tuple_contains_v<MathObjects<type>, ObjectType>
const ObjectType& MathWorld<type>::get(size_t id) const
{
  return std::get<SlottedDeque<ObjectType>>(math_objects).at(id);
}


template <parsing::Type type>
bool MathWorld<type>::contains(std::string_view name) const
{
  return inventory.find(name) != inventory.end();
}


template <parsing::Type type>
MathWorld<type>::ConstDynMathObject MathWorld<type>::to_const(DynMathObject obj) const
{
  return std::visit(
    utils::overloaded{
      [](UnregisteredObject) -> ConstDynMathObject { return UnregisteredObject(); },
      [](auto* val) -> ConstDynMathObject { return val; }
    },
    obj);
}


template <parsing::Type type>
template <class ObjectType, class... Arg>
  requires(tuple_contains_v<MathObjects<type>, ObjectType>
           and (sizeof...(Arg) == 0 or requires(ObjectType o) { o.set(std::declval<Arg>()...); }))
tl::expected<ref<ObjectType>, NameError> MathWorld<type>::add(std::string_view name, Arg &&...arg)
{
  if (not parsing::is_valid_name(name))
    return tl::unexpected(NameError::invalid_format(name));
  else if (contains(name))
    return tl::unexpected(NameError::already_taken(name));

  SlottedDeque<ObjectType> &object_container = std::get<SlottedDeque<ObjectType>>(math_objects);

  size_t id;
  // compile time check if objects needs MathWorld pointer
  if constexpr (requires { ObjectType(this); })
    id = object_container.push(ObjectType(this));
  else id = object_container.push(ObjectType());

  ObjectType& world_object = object_container[id];
  world_object.set_name(std::string(name));
  object_names[&world_object] = name;

  inventory[std::string(name)] = &world_object;

  if constexpr (sizeof...(Arg) > 0)
    world_object.set(std::forward<Arg>(arg)...);

  return world_object;
}

template <parsing::Type type>
tl::expected<Ok, NameError> MathWorld<type>::rename(const std::string& old_name,
                                                    const std::string& new_name)
{
  if (not parsing::is_valid_name(new_name)) [[unlikely]]
    return tl::unexpected(NameError::invalid_format(new_name));

  auto node = inventory.extract(old_name);
  if (node) [[likely]]
  {
    node.key() = new_name;
    std::visit(
      [&]<class T>(T& obj)
      {
        if constexpr (not std::is_same_v<T, UnregisteredObject>)
        {
          obj->set_name(new_name);
          assert(object_names.contains(obj));
          object_names[obj] = new_name;
        }
      },
      node.mapped());

    // put back in the inventory
    inventory.insert(std::move(node));

    // direct dependencies of function objects are update at each parse
    // so functions that depend on "old_name" need to be re-parsed.
    parse_direct_revdeps_of(old_name);

    return Ok{};
  }
  else return tl::unexpected(NameError::not_in_world(old_name));
}

template <parsing::Type type>
template <class ObjectType>
  requires(tuple_contains_v<MathObjects<type>, ObjectType>)
tl::expected<Ok, NameError> MathWorld<type>::set_name(ObjectType* obj, const std::string& name)
{
  const auto it = object_names.find(obj);
  if (it == object_names.end()) [[unlikely]]
    return tl::unexpected(NameError::not_in_world());
  else if (not parsing::is_valid_name(name)) [[unlikely]]
    return tl::unexpected(NameError::invalid_format(name));
  else if (inventory.contains(name))
    return tl::unexpected(NameError::already_taken(name));

  // save old name first
  const std::string old_name = it->second;

  // overwrite with new name
  it->second = name;

  if (not old_name.empty())
  {
    // change the name in the inventory to the new one
    auto node = inventory.extract(old_name);
    assert(bool(node));
    node.key() = name;
    inventory.insert(std::move(node));
  }
  else
  {
    // object didn't have a name before and a name just got assigned to it
    // inventory will
    inventory[name] = obj;
  }

  if (not old_name.empty())
    // if old_name wasn't empty, functions could have been depending on it
    parse_direct_revdeps_of(old_name);

  // functions could be depending on the new name already
  parse_direct_revdeps_of(name);

  obj->set_name(name);

  return Ok{};
}

template <parsing::Type type>
void MathWorld<type>::parse_direct_revdeps_of(const std::string& name)
{
  auto parse_functions = [&]<class T>(SlottedDeque<T>& container)
  {
    if constexpr (is_function_v<T>)
    {
      for (std::optional<T>& obj: container)
      {
        if (obj and obj->direct_dependencies().contains(name))
          obj->parse();
      }
    }
  };
  tuple_for(parse_functions, math_objects);
}

template <parsing::Type type>
tl::expected<double, Error> MathWorld<type>::evaluate(std::string expr) const
{
  Function<type, 0> f(this);
  f.set_expression(std::move(expr));
  return f();
}

} // namespace zc
