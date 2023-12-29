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
    add<CppUnaryFunction<type>>(std::string(name), f_ptr);

  for (auto&& [name, cst]: builtin_global_constants)
    add<GlobalConstant<type>>(std::string(name), cst);
}

template <parsing::Type type>
const MathWorld<type>::DynMathObject* MathWorld<type>::get(std::string_view name) const
{
  auto it = inventory.find(name);
  return it != inventory.end() ? it->second : nullptr;
}

template <parsing::Type type>
MathWorld<type>::DynMathObject* MathWorld<type>::get(std::string_view name)
{
  return const_cast<DynMathObject*>(std::as_const(*this).get(name));
}

template <parsing::Type type>
template <class ObjectType>
  requires tuple_contains_v<MathObjects<type>, ObjectType>
ObjectType* MathWorld<type>::get(std::string_view name)
{
  DynMathObject* dyn_obj = get(name);
  if (dyn_obj and std::holds_alternative<ObjectType>(*dyn_obj))
    return &std::get<ObjectType>(*dyn_obj);
  else return nullptr;
}

template <parsing::Type type>
template <class ObjectType>
  requires tuple_contains_v<MathObjects<type>, ObjectType>
const ObjectType* MathWorld<type>::get(std::string_view name) const
{
  const DynMathObject* dyn_obj = get(name);
  if (dyn_obj and std::holds_alternative<const ObjectType*>(*dyn_obj))
    return &std::get<ObjectType>(*dyn_obj);
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
template <class ObjectType>
  requires(tuple_contains_v<MathObjects<type>, ObjectType>)
ObjectType& MathWorld<type>::add()
{
  size_t id = math_objects.next_free_slot();
  [[maybe_unused]] size_t new_id = math_objects.push(ObjectType(id, this));
  assert(id == new_id); // should  be the same

  ObjectType& world_object = std::get<ObjectType>(math_objects[id]);

  return world_object;
}


template <parsing::Type type>
template <class ObjectType, class... Arg>
  requires(tuple_contains_v<MathObjects<type>, ObjectType>
           and (sizeof...(Arg) == 0 or requires(ObjectType o) { o.set(std::declval<Arg>()...); }))
tl::expected<ref<ObjectType>, Error> MathWorld<type>::add(const std::string& name, Arg &&...arg)
{
  if (not parsing::is_valid_name(name))
    return tl::unexpected(Error::wrong_format(name));
  else if (contains(name))
    return tl::unexpected(Error::name_already_taken(name));

  size_t id = math_objects.next_free_slot();
  [[maybe_unused]] size_t new_id = math_objects.push(ObjectType(id, this));
  assert(id == new_id); // should  be the same

  ObjectType& world_object = std::get<ObjectType>(math_objects[id]);
  world_object.set_name(name);
  object_names[&math_objects[id]] = name;

  inventory[name] = &math_objects[id];

  if constexpr (sizeof...(Arg) > 0)
    world_object.set(std::forward<Arg>(arg)...);

  // some objects may already have expressions depending on this object
  parse_direct_revdeps_of(name);

  return world_object;
}

template <parsing::Type type>
void MathWorld<type>::parse_direct_revdeps_of(const std::string& name)
{
  for (std::optional<DynMathObject>& o: math_objects)
  {
    if (o)
      std::visit(
        [&]<class T>(T& obj) {
          if constexpr (is_function_v<T>)
            if (obj.direct_dependencies().contains(name))
              obj.parse();
        },
        *o);
  }
}

template <parsing::Type type>
tl::expected<double, Error> MathWorld<type>::evaluate(std::string expr)
{
  if (expr.empty()) [[unlikely]]
    return tl::unexpected(Error::empty_expression());

  auto make_uast = [](std::span<const parsing::Token> tokens)
  {
    return parsing::make_uast(tokens);
  };

  auto evaluate = [](const parsing::Parsing<type>& repr)
  {
    return zc::evaluate(repr);
  };

  if constexpr (type == parsing::Type::AST)
    return parsing::tokenize(expr)
      .and_then(make_uast)
      .and_then(parsing::bind<type>{*this})
      .and_then(evaluate);
  else
    return parsing::tokenize(expr)
      .and_then(make_uast)
      .and_then(parsing::bind<type>{*this})
      .transform(parsing::make_RPN)
      .and_then(evaluate);
}

template <parsing::Type type>
template <class ObjectType>
  requires(tuple_contains_v<MathObjects<type>, ObjectType>)
tl::expected<Ok, UnregisteredObject> MathWorld<type>::erase(ObjectType* obj)
{
  if (not math_objects.is_assigned(obj->slot) or
      not std::holds_alternative<ObjectType>(math_objects[obj->slot]) or
      &std::get<ObjectType>(math_objects[obj->slot]) != obj)
    return tl::unexpected(UnregisteredObject{});

  return erase(&math_objects[obj->slot]);
}

template <parsing::Type type>
tl::expected<Ok, UnregisteredObject> MathWorld<type>::erase(DynMathObject* obj)
{
  if (not obj)
    return tl::unexpected(UnregisteredObject{});

  size_t slot = -1;
  std::visit(
    [&](MathObject<type>& math_obj) {
      slot = math_obj.slot;
    },
    *obj);

  assert(slot != size_t(-1));

  if (not math_objects.is_assigned(slot)
      or &math_objects[slot] != obj)
    return tl::unexpected(UnregisteredObject{});

  const auto name_node = object_names.extract(&math_objects[slot]);
  if (bool(name_node))
  {
    // extract "name" from the inventory and just throw it away
    auto node = inventory.extract(name_node.mapped());

    // the inventory *should* have a valid node
    assert(bool(node));

    // functions could be depending on 'name' so re-parse them
    parse_direct_revdeps_of(name_node.mapped());
  }

  // remove math object
  math_objects.pop(slot);

  return Ok{};
}

template <parsing::Type type>
tl::expected<Ok, UnregisteredObject> MathWorld<type>::erase(const std::string& name)
{
  return erase(get(name));
}


} // namespace zc
