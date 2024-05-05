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

#include "zecalculator/parsing/data_structures/decl/ast.h"
#include "zecalculator/parsing/data_structures/token.h"
#include <ranges>
#include <zecalculator/mathworld/decl/mathworld.h>
#include <zecalculator/math_objects/impl/dyn_math_object.h>
#include <zecalculator/parsing/parser.h>

namespace zc {

template <parsing::Type type>
MathWorld<type>::MathWorld()
{
  for (auto&& [name, f_ptr]: builtin_unary_functions)
    add<1>(std::string(name), f_ptr);

  for (auto&& equation: builtin_global_constants)
    add<GlobalConstant<type>>(std::string(equation));
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
DynMathObject<type>& MathWorld<type>::add()
{
  return add(math_objects.next_free_slot());
}

template <parsing::Type type>
DynMathObject<type>& MathWorld<type>::add(size_t slot)
{
  math_objects.push(DynMathObject<type>(tl::unexpected(Error::empty_expression()),
                                        MathWorldObjectHandle<type>{slot, this}),
                    slot);

  return math_objects[slot];
}

template <parsing::Type type>
template <class InterpretAs>
  requires (tuple_contains_v<MathEqObjects<type>, InterpretAs>
            or std::is_same_v<DynMathObject<type>, InterpretAs>)
DynMathObject<type>& MathWorld<type>::add(std::string definition)
{
  return add<InterpretAs>(definition, math_objects.next_free_slot());
}

template <parsing::Type type>
template <class InterpretAs>
  requires (tuple_contains_v<MathEqObjects<type>, InterpretAs>
            or std::is_same_v<DynMathObject<type>, InterpretAs>)
DynMathObject<type>& MathWorld<type>::add(std::string definition, size_t slot)
{
 /**
  *  0. Example: def = "f(x) = cos(x)"
  *  1. Tokenize the definition: ['f', '(', 'x', ')', '=', 'cos', '(', 'x', ')']
  *  2. Make AST representation of the definition
  *         =
  *        / \
  *       f  cos
  *      /     \
  *     x       x
  *  3. Extract information
  *    - Function name
  *    - Variable name(s)
  *    - Expression
  *  4. Any step can fail, in which case the member variable 'm' is an Error instance
  **/

  auto& obj = add(slot);

  MathEqObject<type> math_expr_obj(obj, definition);

  // sanity checks
  if (not sanity_check(obj))
  {
    obj = tl::unexpected(Error::object_not_in_world());
    return obj;
  }

  auto ast = parsing::tokenize(definition).and_then(parsing::make_ast{definition});
  if (not ast)
  {
    obj = tl::unexpected(ast.error());
    return obj;
  }

  // the root node of the tree must be the equal sign
  if (not (ast->is_func() and ast->func_data().type == parsing::AST::Func::OP_ASSIGN))
  {
    obj = tl::unexpected(Error::not_math_object_definition());
    return obj;
  }

  auto& funcop_data = ast->func_data();

  assert(funcop_data.subnodes.size() == 2);

  math_expr_obj.lhs = std::move(funcop_data.subnodes[0]);
  math_expr_obj.rhs = std::move(funcop_data.subnodes[1]);

  funcop_data.subnodes.clear();

  // "f(x) = ...."
  const bool is_function_def = (math_expr_obj.lhs.is_func()
                                and math_expr_obj.lhs.func_data().type
                                      == parsing::AST::Func::FUNCTION);

  // "var = complex expression that is not a number"
  const bool is_global_var_def = (math_expr_obj.lhs.is_var()
                                 and not math_expr_obj.rhs.is_number());

  // "var = 13.24213 (a number)"
  const bool is_global_constant_def = (math_expr_obj.lhs.is_var() and math_expr_obj.rhs.is_number());

  if (is_function_def or is_global_var_def)
  {
    math_expr_obj.name = math_expr_obj.lhs.name;

    if (is_global_var_def)
    {
      obj = Function<type>(math_expr_obj, {});
    }
    else // is_function_def
    {
      const std::vector<parsing::AST>& args = math_expr_obj.lhs.func_data().subnodes;

      std::vector<parsing::tokens::Text> var_name_tokens;
      var_name_tokens.reserve(args.size());

      // setting up the text that contains all the args
      parsing::tokens::Text args_txt = math_expr_obj.lhs.args_token();

      // the arguments of the function call in the left hand-side must all be regular variables
      for (const auto& arg: args)
      {
        if (not arg.is_var())
        {
          obj = tl::unexpected(Error::unexpected(arg.name, definition));
          return obj;
        }
        var_name_tokens.push_back(arg.name);
      }

      auto var_name_strs = var_name_tokens | std::views::transform(&parsing::tokens::Text::substr);

      // override 'rhs' with input variables of the function properly marked
      math_expr_obj.rhs = parsing::mark_input_vars{var_name_strs}(math_expr_obj.rhs);

      if constexpr (not zc::is_sequence_v<InterpretAs>)
        obj = Function<type>(math_expr_obj, std::move(var_name_tokens));
      else
      {
        if (args.size() != 1)
        {
          obj = tl::unexpected(Error::mismatched_fun_args(args_txt, definition));
          return obj;
        }

        obj = Sequence(Function<type>(math_expr_obj, std::move(var_name_tokens)));
      }
    }
  }
  else if (is_global_constant_def)
  {
    math_expr_obj.name = math_expr_obj.lhs.name;
    obj = GlobalConstant<type>(math_expr_obj,
                               parsing::Token(math_expr_obj.rhs.number_data().value,
                                              math_expr_obj.rhs.name));
  }
  else
  {
    obj = tl::unexpected(Error::unexpected(math_expr_obj.lhs.name, definition));
    return obj;
  }

  if constexpr (not std::is_same_v<InterpretAs, DynMathObject<type>>)
  {
    if (not obj.template holds<InterpretAs>())
    {
      // caller of function asked for interpreting the object as another type
      obj = tl::unexpected(Error::wrong_object_type(math_expr_obj.name, definition));
      return obj;
    }
  }

  if (inventory.contains(math_expr_obj.name.substr))
  {
    obj = tl::unexpected(Error::name_already_taken(math_expr_obj.name, definition));
    return obj;
  }

  inventory[math_expr_obj.name.substr] = slot;
  object_names.push(math_expr_obj.name.substr, slot);

  rebind_direct_revdeps_of(math_expr_obj.name.substr);

  return obj;
}

template <parsing::Type type>
template <size_t args_num>
  requires (args_num <= max_func_args)
DynMathObject<type>& MathWorld<type>::add(std::string name, CppMathFunctionPtr<args_num> cpp_f)
{
  return add<args_num>(std::move(name), cpp_f, math_objects.next_free_slot());
}

template <parsing::Type type>
template <size_t args_num>
  requires (args_num <= max_func_args)
DynMathObject<type>& MathWorld<type>::add(std::string name, CppMathFunctionPtr<args_num> cpp_f, size_t slot)
{
  auto& obj = add(slot);

  if (inventory.contains(name))
  {
    obj = tl::unexpected(Error::name_already_taken(parsing::tokens::Text(name), ""));
    return obj;
  }

  object_names.push(name, slot);
  inventory[name] = slot;

  obj = CppFunction<type, args_num>({name, obj}, cpp_f);
  return obj;
}

template <parsing::Type type>
template <class InterpretAs>
  requires (tuple_contains_v<MathEqObjects<type>, InterpretAs>
            or std::is_same_v<DynMathObject<type>, InterpretAs>)
DynMathObject<type>& MathWorld<type>::redefine(DynMathObject<type>& obj, std::string definition)
{
  if (not sanity_check(obj))
    throw  std::runtime_error("Object not registered in this world");

  const size_t slot = obj.slot;
  erase(obj);
  auto& new_obj = add<InterpretAs>(definition, slot);
  assert(&new_obj == &obj);
  return obj;
}

template <parsing::Type type>
bool MathWorld<type>::sanity_check(const DynMathObject<type>& obj)
{
  return obj.mathworld == this
        and math_objects.is_assigned(obj.slot)
        and &math_objects[obj.slot] == &obj;
}

template <parsing::Type type>
template <size_t args_num>
  requires (args_num <= max_func_args)
DynMathObject<type>& MathWorld<type>::redefine(DynMathObject<type>& obj,
                                               std::string name,
                                               CppMathFunctionPtr<args_num> cpp_f)
{
  if (not sanity_check(obj))
    throw std::runtime_error("Object not in this world");

  size_t slot = obj.slot;
  erase(obj);
  DynMathObject<type>& new_obj = add(name, cpp_f);
  assert(new_obj.slot == slot);

  return new_obj;
}

template <parsing::Type type>
deps::Deps MathWorld<type>::direct_revdeps(std::string_view name) const
{
  deps::Deps direct_rev_deps;
  for (const std::optional<DynMathObject<type>>& o: math_objects)
  {
    if (o and o->has_value())
      std::visit(
        [&]<class T>(const T& obj) {
          if constexpr (is_function_v<T>)
            if (auto deps = obj.direct_dependencies().contains(name))
              direct_rev_deps.insert({obj.get_name(), deps::FUNCTION});
        },
        **o);
  }
  return direct_rev_deps;
}

template <parsing::Type type>
void MathWorld<type>::rebind_direct_revdeps_of(const std::string& name)
{
  const auto direct_rev_deps = direct_revdeps(name);
  for (auto&&[obj_name, obj_type]: direct_rev_deps)
  {
    if (DynMathObject<type>* obj = get(obj_name))
      if (obj->has_value())
        std::visit(
          [&]<class T>(T& v)
          {
            if constexpr (is_function_v<T>)
              v.rebind();
          },
          **obj);
  }
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
      .and_then(parsing::make_fast<type>{expr, *this})
      .and_then(evaluate);
  else
    return parsing::tokenize(expr)
      .and_then(parsing::make_ast{expr})
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

  if (object_names.is_assigned(slot))
  {
    const std::string name = object_names[slot];

    const size_t erased_num = inventory.erase(name);
    assert(erased_num);

    object_names.free(slot);

    rebind_direct_revdeps_of(name);
  }

  math_objects.free(slot);

  return Ok{};
}

} // namespace zc
