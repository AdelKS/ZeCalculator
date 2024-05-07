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

  eq_functions.free(slot);

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

  // first equation sanity check
  if (not is_function_def and not is_global_var_def and not is_global_constant_def) [[unlikely]]
  {
    obj = tl::unexpected(Error::unexpected(math_expr_obj.lhs.name, definition));
    return obj;
  }

  // second sanity check:
  // if the left side of the equation is a function call
  // it should only contain variables in each of its arguments, e.g. "f(x, y)"
  // and not e.g. "f(x^2 + 1, x + y)"
  if (is_function_def)
    for (const auto& arg: math_expr_obj.lhs.func_data().subnodes)
      if (not arg.is_var())
      {
        obj = tl::unexpected(Error::unexpected(arg.name, definition));
        return obj;
      }

  // third sanity check: type checks
  if constexpr (not std::is_same_v<InterpretAs, DynMathObject<type>>)
  {
    if ( (is_function_def or is_global_var_def) )
    {
      const std::vector<parsing::AST>& args = math_expr_obj.lhs.func_data().subnodes;

      if (not is_function_v<InterpretAs>
          or (zc::is_sequence_v<InterpretAs> and args.size() == 0)) [[unlikely]]
      {
        obj = tl::unexpected(Error::wrong_object_type(math_expr_obj.lhs.name, definition));
        return obj;
      }
      else if ( zc::is_sequence_v<InterpretAs> and args.size() != 1 )
      {
        obj = tl::unexpected(Error::mismatched_fun_args(math_expr_obj.lhs.args_token(), definition));
        return obj;
      }
    }
    else if (is_global_constant_def and not std::is_same_v<InterpretAs, GlobalConstant<type>>)
    {
      obj = tl::unexpected(Error::wrong_object_type(math_expr_obj.lhs.name, definition));
      return obj;
    }
  }

  // fourth sanity check: name check
  if (inventory.contains(math_expr_obj.lhs.name.substr))
  {
    obj = tl::unexpected(Error::name_already_taken(math_expr_obj.lhs.name, definition));
    return obj;
  }

  math_expr_obj.name = math_expr_obj.lhs.name.substr;

  std::vector<std::string> var_names;

  if (is_function_def)
  {
    // fill 'arg_names'
    const std::vector<parsing::AST>& args = math_expr_obj.lhs.func_data().subnodes;

    var_names.reserve(args.size());

    // the arguments of the function call in the left hand-side must all be regular variables
    for (const auto& arg: args)
      var_names.push_back(arg.name.substr);

    // mark function's input variables in 'rhs'
    math_expr_obj.rhs = parsing::mark_input_vars{var_names}(math_expr_obj.rhs);
  }

  // now that we checked that everything is fine, we can assign the object
  if (is_function_def or is_global_var_def)
  {
    if constexpr (not zc::is_sequence_v<InterpretAs>)
      eq_functions.push({math_expr_obj, var_names.size(), EqFunction::FUNCTION}, slot);
    else
      eq_functions.push({math_expr_obj, var_names.size(), EqFunction::SEQUENCE}, slot);
  }
  else
  {
    assert(is_global_constant_def);
    obj = GlobalConstant<type>(math_expr_obj,
                               parsing::Token(math_expr_obj.rhs.number_data().value,
                                              math_expr_obj.rhs.name));

    inventory[math_expr_obj.name] = slot;
    object_names.push(math_expr_obj.name, slot);
  }

  rebind_functions();

  return obj;
}

template <parsing::Type type>
void MathWorld<type>::rebind_functions()
{
  for (const EqFunction& eq_f: eq_functions)
  {
    const size_t slot = eq_f.eq_obj.slot;
    if (not math_objects.is_assigned(slot) or not math_objects[slot].has_value())
    {
      if (eq_f.obj_type == EqFunction::FUNCTION)
        math_objects.push(DynMathObject<type>(Function<type>(eq_f.eq_obj, eq_f.args_num),
                                              MathWorldObjectHandle<type>{slot, this}),
                          slot);
      else
        math_objects.push(DynMathObject<type>(Sequence<type>(
                                                Function<type>(eq_f.eq_obj, eq_f.args_num)),
                                              MathWorldObjectHandle<type>{slot, this}),
                          slot);

      inventory[eq_f.eq_obj.name] = slot;
      object_names.push(eq_f.eq_obj.name, slot);
    }
  }

  auto get_final_representation = [this](const EqFunction& eqf)
  {
    if constexpr (type == parsing::Type::FAST)
      return parsing::make_fast<type>{eqf.eq_obj.m_equation, *this}(eqf.eq_obj.rhs);
    else
      return parsing::make_fast<type>{eqf.eq_obj.m_equation, *this}(eqf.eq_obj.rhs).transform(parsing::make_RPN);
  };

  std::stack<std::string> invalid_functions;

  for (EqFunction& eq_f: eq_functions)
  {
    const size_t slot = eq_f.eq_obj.slot;

    // should be assigned in the previous loop
    assert(math_objects[slot].has_value());

    std::visit(
      utils::overloaded{
        [&]<class T>(T& f)
        {
          constexpr bool func_or_seq = std::is_same_v<T, Function<type>> or std::is_same_v<T, Sequence<type>>;

          // on runtime, we should only match against functions and sequences
          assert(func_or_seq);

          // at compile time, we don't care about anything else
          if constexpr (func_or_seq)
          {
            // if it's a valid function or sequence,
            // they must have a slot in 'eq_functions'
            assert(eq_functions.is_assigned(slot));

            if (auto exp_rhs = get_final_representation(eq_f))
              f.bound_rhs = std::move(*exp_rhs);
            else
            {
              assert(eq_f.eq_obj.name == f.name);
              invalid_functions.push(eq_f.eq_obj.name);
              math_objects[slot] = tl::unexpected(std::move(exp_rhs.error()));
            }
          }
        },
      },
      *math_objects[slot]
    );
  }

  std::unordered_set<std::string> covered_invalid_functions;

  while(not invalid_functions.empty())
  {
    std::string invalid_func_name = invalid_functions.top();
    covered_invalid_functions.insert(invalid_func_name);
    invalid_functions.pop();

    if (auto it = inventory.find(invalid_func_name); it != inventory.end())
    {
      object_names.free(it->second);
      inventory.erase(it);
    }

    auto revdeps = direct_revdeps(invalid_func_name);
    for (auto&& [affected_func_name, info]: revdeps)
    {
      if(DynMathObject<type>* obj = get(affected_func_name))
      {
        // if these are revdeps, they can only be functions
        // because they have an expression that calls our invalid function
        assert(obj->template holds<Function<type>>() or obj->template holds<Sequence<type>>());

        assert(eq_functions.is_assigned(obj->slot));

        *obj = tl::unexpected(
          Error::object_in_invalid_state(parsing::tokens::Text{.substr = invalid_func_name,
                                                               .begin = info.indexes.front()},
                                         eq_functions[obj->slot].eq_obj.m_equation));

        invalid_functions.push(affected_func_name);
      }
    }
  }
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
    obj = tl::unexpected(Error::name_already_taken(name));
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
  for (const DynMathObject<type>& o: math_objects)
  {
    if (o.has_value())
      std::visit(
        [&]<class T>(const T& obj) {
          if constexpr (is_function_v<T>)
          {
            auto deps = obj.direct_dependencies();
            if (auto it = deps.find(name); it != deps.end())
            {
              deps::Dep& dep = direct_rev_deps[obj.get_name()];
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
  }

  eq_functions.free(slot);
  math_objects.free(slot);

  rebind_functions();

  return Ok{};
}

} // namespace zc
