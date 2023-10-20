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
  : MathWorld<type>::MathWorld(builtin_unary_functions, builtin_global_constants){};


template <parsing::Type type>
template <class ObjectType1, size_t size1, class... ObjectTypeN, size_t... sizeN>
MathWorld<type>::MathWorld(const std::array<std::pair<std::string_view, ObjectType1>, size1>& objects1,
            const std::array<std::pair<std::string_view, ObjectTypeN>, sizeN>&... objectsN)
  : MathWorld(objectsN...)
{
  for(auto [name, obj]: objects1)
    add<ObjectType1>(name, obj);
}

template <parsing::Type type>
template <class ObjectType, size_t size>
MathWorld<type>::MathWorld(const std::array<std::pair<std::string_view, ObjectType>, size>& objects)
{
  for(auto [name, obj]: objects)
    add<ObjectType>(name, obj);
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
    overloaded{
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

  inventory[std::string(name)] = &world_object;

  if constexpr (sizeof...(Arg) > 0)
    world_object.set(std::forward<Arg>(arg)...);

  return world_object;
}


template <parsing::Type type>
tl::expected<double, Error> MathWorld<type>::evaluate(std::string expr) const
{
  Function<type, 0> f(this);
  f.set_expression(std::move(expr));
  return f();
}

} // namespace zc
