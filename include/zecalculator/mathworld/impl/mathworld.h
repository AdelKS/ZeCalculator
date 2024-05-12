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
#include <zecalculator/mathworld/decl/mathworld.h>
#include <zecalculator/parsing/data_structures/decl/ast.h>
#include <zecalculator/parsing/data_structures/token.h>
#include <zecalculator/parsing/parser.h>

#include <cassert>
#include <stack>
#include <unordered_set>

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
const DynMathObject<type>* MathWorld<type>::get(std::string_view name) const
{
  auto it = inventory.find(name);
  return it != inventory.end() ? &math_objects[it->second] : nullptr;
}

template <parsing::Type type>
DynMathObject<type>* MathWorld<type>::get(std::string_view name)
{
  return const_cast<DynMathObject<type>*>(std::as_const(*this).get(name));
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

  math_objects.push(DynMathObject<type>(tl::unexpected(Error::empty_expression()), slot, this), slot);

  return math_objects[slot];
}

template <parsing::Type type>
void MathWorld<type>::rebind_functions()
{
  std::vector<size_t> func_slots;
  std::ranges::copy_if(std::views::iota(0ul, math_objects.size()),
                       std::back_inserter(func_slots),
                       [&](size_t slot)
                       {
                         return math_objects.is_assigned(slot) and math_objects[slot].opt_eq_object
                                and (math_objects[slot].opt_eq_object->cat == EqObject::FUNCTION
                                     or math_objects[slot].opt_eq_object->cat == EqObject::SEQUENCE);
                       });

  for (size_t slot: func_slots)
  {
    DynMathObject<type>& dyn_obj = math_objects[slot];

    assert(dyn_obj.opt_eq_object);

    const EqObject& eq_obj = *dyn_obj.opt_eq_object;

    if (not math_objects.is_assigned(slot) or not math_objects[slot].has_value())
    {
      if (eq_obj.cat == EqObject::FUNCTION)
        dyn_obj.as_expected() = Function<type>(eq_obj.name, eq_obj.lhs.args_num());
      else if (eq_obj.cat == EqObject::SEQUENCE)
        dyn_obj.as_expected() = Sequence<type>(eq_obj.name);
      else assert(false);

      inventory[eq_obj.name] = slot;
    }
  }

  auto get_final_representation = [this](const std::string& eq, const parsing::AST& ast)
  {
    if constexpr (type == parsing::Type::FAST)
      return parsing::make_fast<type>{eq, *this}(ast);
    else
      return parsing::make_fast<type>{eq, *this}(ast).transform(parsing::make_RPN);
  };

  std::unordered_set<std::string> invalid_functions;

  for (size_t slot: func_slots)
  {
    DynMathObject<type>& dyn_obj = math_objects[slot];

    assert(dyn_obj.opt_eq_object);

    const EqObject& eq_obj = *dyn_obj.opt_eq_object;

    // should be assigned in the previous loop
    assert(dyn_obj.has_value());

    std::visit(
      utils::overloaded{
        [&](Function<type>& f)
        {
          assert(eq_obj.name == f.name);

          if (auto exp_rhs = get_final_representation(eq_obj.equation, eq_obj.rhs))
            f.bound_rhs = std::move(*exp_rhs);
          else
          {
            invalid_functions.insert(eq_obj.name);
            dyn_obj.as_expected() = tl::unexpected(std::move(exp_rhs.error()));
          }
        },
        [&](Sequence<type>& u)
        {
          assert(eq_obj.name == u.name);
          std::vector<const parsing::AST*> seq_ast_values;

          if (eq_obj.rhs.is_func() and eq_obj.rhs.func_data().type == parsing::AST::Func::SEPARATOR)
          {
            // sequence defined with first values
            seq_ast_values.reserve(eq_obj.rhs.func_data().subnodes.size());
            std::ranges::transform(eq_obj.rhs.func_data().subnodes,
                                   std::back_inserter(seq_ast_values),
                                   [](auto&& val){ return &val; });
          }
          // sequence defined without first values
          else seq_ast_values.push_back(&eq_obj.rhs);

          u.values.reserve(seq_ast_values.size());
          for (const parsing::AST* ast: seq_ast_values)
            if (auto exp_rhs = get_final_representation(eq_obj.equation, *ast))
              u.values.push_back(std::move(*exp_rhs));
            else
            {
              invalid_functions.insert(eq_obj.name);
              dyn_obj.as_expected() = tl::unexpected(std::move(exp_rhs.error()));
              break;
            }
        },
        [](auto&&) { assert(false); /* we are not supposed to ever hit here */}
      },
      *dyn_obj
    );
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
deps::Deps MathWorld<type>::direct_dependencies(const DynMathObject<type>& obj) const
{
  if (not sanity_check(obj)) [[unlikely]]
    throw std::runtime_error("Object doesn't belong to math world");

  return direct_dependencies(obj.slot);
}

template <parsing::Type type>
void MathWorld<type>::name_change(size_t slot, std::string_view old_name, std::string_view new_name)
{
  if (not old_name.empty())
  {
    auto it = inventory.find(old_name);
    assert(it != inventory.end());
    inventory.erase(it);
  }

  if (not new_name.empty())
    inventory[std::string(new_name)] = slot;
}

template <parsing::Type type>
deps::Deps MathWorld<type>::direct_dependencies(std::string_view name) const
{
  if(auto* obj = get(name))
    return direct_dependencies(*obj);
  else return deps::Deps();
}

template <parsing::Type type>
deps::Deps MathWorld<type>::direct_dependencies(size_t slot) const
{
  if (not math_objects.is_assigned(slot) or not math_objects[slot].opt_eq_object)
    return deps::Deps();

  return parsing::direct_dependencies(math_objects[slot].opt_eq_object->rhs);
}

template <parsing::Type type>
deps::Deps MathWorld<type>::direct_revdeps(std::string_view name) const
{
  deps::Deps direct_rev_deps;
  for (const DynMathObject<type>& o: math_objects)
  {
    if (o.has_value())
      std::visit(
        [&]<class T>(const T& obj) {
          if constexpr (is_function_v<T>)
          {
            auto deps = direct_dependencies(o.slot);
            if (auto it = deps.find(name); it != deps.end())
            {
              deps::Dep& dep = direct_rev_deps[std::string(obj.get_name())];
              dep.type = deps::Dep::FUNCTION;
              dep.indexes = it->second.indexes;
            }
          }
        },
        *o);
  }
  return direct_rev_deps;
}

template <parsing::Type type>
tl::expected<double, Error> MathWorld<type>::evaluate(std::string expr)
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

  if (std::string_view name = math_objects[slot].get_name(); not name.empty())
  {
    auto it = inventory.find(name);
    assert(it != inventory.end());
    inventory.erase(it);
  }

  math_objects.free(slot);

  rebind_functions();

  return Ok{};
}

} // namespace zc
