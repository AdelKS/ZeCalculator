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

#include <zecalculator/math_objects/impl/dyn_math_object.h>
#include <zecalculator/math_objects/impl/eq_object.h>
#include <zecalculator/mathworld/decl/mathworld.h>
#include <zecalculator/parsing/data_structures/decl/ast.h>
#include <zecalculator/parsing/data_structures/token.h>
#include <zecalculator/parsing/parser.h>

#include <cassert>

namespace zc {

template <parsing::Type type>
MathWorld<type>::MathWorld()
{
  for (auto&& cpp_f: builtin_unary_functions)
    new_object() = cpp_f;

  for (auto&& cst: builtin_global_constants)
    new_object() = cst;
}

template <parsing::Type type>
MathWorld<type>::iterator MathWorld<type>::begin()
{
  return math_objects.begin();
}

template <parsing::Type type>
MathWorld<type>::const_iterator MathWorld<type>::begin() const
{
  return math_objects.begin();
}

template <parsing::Type type>
MathWorld<type>::const_iterator MathWorld<type>::cbegin() const
{
  return math_objects.cbegin();
}

template <parsing::Type type>
MathWorld<type>::iterator MathWorld<type>::end()
{
  return math_objects.end();
}

template <parsing::Type type>
MathWorld<type>::const_iterator MathWorld<type>::end() const
{
  return math_objects.end();
}

template <parsing::Type type>
MathWorld<type>::const_iterator MathWorld<type>::cend() const
{
  return math_objects.cend();
}

template <parsing::Type type>
const DynMathObject<type>* MathWorld<type>::get(std::string_view name) const
{
  auto it = inventory.find(name);
  return it != inventory.end() ? &math_objects[it->second] : nullptr;
}

template <parsing::Type type>
const DynMathObject<type>* MathWorld<type>::eq_object_get(std::string_view name) const
{
  auto it = eq_object_inventory.find(name);
  return it != eq_object_inventory.end() ? &math_objects[it->second] : nullptr;
}

template <parsing::Type type>
DynMathObject<type>* MathWorld<type>::get(std::string_view name)
{
  return const_cast<DynMathObject<type>*>(std::as_const(*this).get(name));
}

template <parsing::Type type>
DynMathObject<type>* MathWorld<type>::eq_object_get(std::string_view name)
{
  return const_cast<DynMathObject<type>*>(std::as_const(*this).eq_object_get(name));
}

template <parsing::Type type>
template <class ObjectType>
  requires tuple_contains_v<MathObjects<type>, ObjectType>
ObjectType* MathWorld<type>::get(std::string_view name)
{
  DynMathObject<type>* dyn_obj = get(name);

  if (dyn_obj and dyn_obj->template holds<ObjectType>())
    return &dyn_obj->template value_as<ObjectType>();
  else return nullptr;
}

template <parsing::Type type>
template <class ObjectType>
  requires tuple_contains_v<MathObjects<type>, ObjectType>
const ObjectType* MathWorld<type>::get(std::string_view name) const
{
  const DynMathObject<type>* dyn_obj = get(name);
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
DynMathObject<type>& MathWorld<type>::new_object()
{
  size_t slot = math_objects.next_free_slot();

  math_objects.push(DynMathObject<type>(tl::unexpected(Error::empty_expression()), slot, *this), slot);

  return math_objects[slot];
}

template <parsing::Type type>
std::unordered_set<DynMathObject<type>*>
  MathWorld<type>::dependent_eq_objects(const std::unordered_set<std::string>& names)
{
  std::unordered_set<DynMathObject<type>*> dep_eq_objs;

  std::unordered_set<std::string> dep_names;

  for(const std::string& name: names)
    for(auto&& [dep_name, info]: direct_revdeps(name))
      dep_names.insert(dep_name);

  std::unordered_set<std::string> explored_deps;

  while(not dep_names.empty())
  {
    std::string name = *dep_names.begin();
    dep_names.erase(dep_names.begin());

    explored_deps.insert(name);

    if (DynMathObject<type>* obj = eq_object_get(name))
    {
      // should have an EqObject assigned of Function/Sequence type
      // otherwise it cannot depend on anything
      assert(obj->has_function_eq_obj());
      dep_eq_objs.insert(obj);
    }

    for(auto&& [dep_name, info]: direct_revdeps(name))
      if (not explored_deps.contains(dep_name))
        dep_names.insert(dep_name);
  }

  return dep_eq_objs;
}

template <parsing::Type type>
void MathWorld<type>::rebind_dependent_functions(const std::unordered_set<std::string>& names)
{
  std::unordered_set<DynMathObject<type>*> dep_eq_objs = dependent_eq_objects(names);

  for (DynMathObject<type>* dyn_obj: dep_eq_objs)
  {
    assert(dyn_obj);
    assert(dyn_obj->opt_eq_object);

    const EqObject& eq_obj = *dyn_obj->opt_eq_object;

    /// activate if in error state, because its EqObject is actually valid
    /// the function/sequence freshly created does not have a parsing
    if (not dyn_obj->has_value())
    {
      assert(not inventory.contains(eq_obj.name));
      assert(bool(dyn_obj->opt_eq_object));

      dyn_obj->as_expected() = dyn_obj->opt_eq_object->template to_expected_unbound<type>();
      inventory[eq_obj.name] = dyn_obj->slot;
    }
  }

  std::unordered_set<std::string> invalid_functions;

  for (DynMathObject<type>* dyn_obj: dep_eq_objs)
  {
    dyn_obj->as_expected() = dyn_obj->opt_eq_object->template to_expected<type>(*this);
    if (not dyn_obj->has_value())
      invalid_functions.insert(dyn_obj->opt_eq_object->name);
  }

  std::unordered_set<std::string> covered_invalid_functions;

  while(not invalid_functions.empty())
  {
    std::string invalid_func_name = *invalid_functions.begin();
    invalid_functions.erase(invalid_functions.begin());

    if (covered_invalid_functions.contains(invalid_func_name))
      continue;

    inventory.erase(invalid_func_name);

    covered_invalid_functions.insert(invalid_func_name);

    auto revdeps = direct_revdeps(invalid_func_name);
    for (auto&& [affected_func_name, info]: revdeps)
    {
      if(DynMathObject<type>* obj = get(affected_func_name))
      {
        if (covered_invalid_functions.contains(affected_func_name))
          continue;

        // if these are revdeps, they can only be functions
        // because they have an expression that calls our invalid function
        assert(obj->template holds<Function<type>>() or obj->template holds<Sequence<type>>());

        assert(obj->opt_eq_object);

        obj->as_expected() = tl::unexpected(
          Error::object_in_invalid_state(parsing::tokens::Text{.substr = invalid_func_name,
                                                               .begin = info.indexes.front()},
                                         obj->opt_eq_object->equation));

        invalid_functions.insert(affected_func_name);
      }
    }
  }
}

template <parsing::Type type>
bool MathWorld<type>::sanity_check(const DynMathObject<type>& obj) const
{
  return math_objects.is_assigned(obj.slot) and &math_objects[obj.slot] == &obj;
}

template <parsing::Type type>
void MathWorld<type>::object_updated(size_t slot,
                                     bool is_eq_object_now,
                                     std::string old_name,
                                     std::string new_name)
{
  if (not old_name.empty())
  {
    inventory.erase(old_name);
    eq_object_inventory.erase(old_name);
  }

  if (is_eq_object_now and not new_name.empty())
    eq_object_inventory[new_name] = slot;

  if (not new_name.empty() and math_objects[slot].has_value())
    inventory[new_name] = slot;

  rebind_dependent_functions({old_name, new_name});
}

template <parsing::Type type>
deps::Deps MathWorld<type>::direct_dependencies(std::string_view name) const
{
  if(auto* obj = get(name))
    return obj->direct_dependencies();
  else return deps::Deps();
}

template <parsing::Type type>
deps::Deps MathWorld<type>::direct_revdeps(std::string_view name) const
{
  deps::Deps direct_rev_deps;
  for (auto&& [obj_name, slot]: eq_object_inventory)
  {
    assert(math_objects[slot].opt_eq_object);
    assert(math_objects[slot].opt_eq_object->name == obj_name);

    auto deps = math_objects[slot].direct_dependencies();
    if (auto it = deps.find(name); it != deps.end())
    {
      deps::Dep& dep = direct_rev_deps[obj_name];
      dep.type = deps::Dep::FUNCTION;
      dep.indexes = it->second.indexes;
    }
  }
  return direct_rev_deps;
}

template <parsing::Type type>
tl::expected<double, Error> MathWorld<type>::evaluate(std::string expr) const
{
  if (expr.empty()) [[unlikely]]
    return tl::unexpected(Error::empty_expression());

  auto evaluate = [](const parsing::Parsing<type>& repr)
  {
    return zc::evaluate(repr);
  };

  if constexpr (type == parsing::Type::FAST)
    return parsing::tokenize(expr)
      .and_then(parsing::make_ast{expr})
      .transform(parsing::flatten_separators)
      .and_then(parsing::make_fast<type>{expr, *this})
      .and_then(evaluate);
  else
    return parsing::tokenize(expr)
      .and_then(parsing::make_ast{expr})
      .transform(parsing::flatten_separators)
      .and_then(parsing::make_fast<type>{expr, *this})
      .transform(parsing::make_RPN)
      .and_then(evaluate);
}

template <parsing::Type type>
template <class ObjectType>
  requires(tuple_contains_v<MathObjects<type>, ObjectType>)
tl::expected<Ok, UnregisteredObject> MathWorld<type>::erase(ObjectType& obj)
{
  if (obj.slot >= math_objects.size()
      or not math_objects.is_assigned(obj.slot)
      or not bool(math_objects[obj.slot])
      or std::get_if<ObjectType>(&math_objects[obj.slot].value()) != &obj)
    return tl::unexpected(UnregisteredObject{});

  return erase(obj.slot);
}

template <parsing::Type type>
tl::expected<Ok, UnregisteredObject> MathWorld<type>::erase(DynMathObject<type>& obj)
{
  if (not sanity_check(obj))
    return tl::unexpected(UnregisteredObject{});

  return erase(obj.slot);
}

template <parsing::Type type>
tl::expected<Ok, UnregisteredObject> MathWorld<type>::erase(const std::string& name)
{
  auto it = inventory.find(name);
  if (it != inventory.end())
    return erase(it->second);
  else return tl::unexpected(UnregisteredObject{});
}

template <parsing::Type type>
tl::expected<Ok, UnregisteredObject> MathWorld<type>::erase(size_t slot)
{
  if (not math_objects.is_assigned(slot))
    return tl::unexpected(UnregisteredObject{});

  std::string name(math_objects[slot].get_name());

  math_objects.free(slot);

  if (auto it = eq_object_inventory.find(name); it != eq_object_inventory.end())
    eq_object_inventory.erase(it);

  object_updated(slot, false, name, "");

  return Ok{};
}

} // namespace zc
