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

#include <zecalculator/mathworld/decl/mathworld_template.h>
#include <zecalculator/parsing/parser.h>

namespace zc {

template <class... MathObjectType>
template <class ObjectType>
tl::expected<ref<ObjectType>, NameError> MathWorldT<MathObjectType...>::add(std::string_view name)
{
  if (not parsing::is_valid_name(name))
    return tl::unexpected(NameError::invalid_format(name));
  else if (contains(name))
    return tl::unexpected(NameError::already_taken(name));

  SlottedDeque<ObjectType> &object_container = std::get<SlottedDeque<ObjectType>>(math_objects);

  size_t id;
  // compile time check if objects needs MathWorld pointer
  if constexpr (std::is_constructible_v<ObjectType, const MathWorldT<MathObjectType...>*>)
    id = object_container.push(ObjectType(this)); // TODO: define emplace_back in SlottedVector
  else id = object_container.push(ObjectType());

  ObjectType& world_object = object_container[id];
  inventory[std::string(name)] = std::ref(world_object);
  return world_object;
}

template <class... MathObjectType>
template <class ObjectType, class... Args>
tl::expected<ref<ObjectType>, NameError> MathWorldT<MathObjectType...>::add(std::string_view name, Args&&... args)
{
  if (not parsing::is_valid_name(name))
    return tl::unexpected(NameError::invalid_format(name));
  else if (contains(name))
    return tl::unexpected(NameError::already_taken(name));

  SlottedDeque<ObjectType> &object_container = std::get<SlottedDeque<ObjectType>>(math_objects);

  size_t id;
  // compile time check if objects needs MathWorld cref
  if constexpr (std::is_constructible_v<ObjectType, Args..., const MathWorldT<MathObjectType...>*>)
    id = object_container.push(ObjectType(std::forward<Args>(args)..., this)); // TODO: define emplace_back in SlottedVector
  else id = object_container.push(ObjectType(std::forward<Args>(args)...));

  ObjectType& world_object = object_container[id];
  inventory[std::string(name)] = std::ref(world_object);
  return world_object;
}

}
