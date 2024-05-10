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
  math_objects.push(DynMathObject<type>(tl::unexpected(Error::empty_expression()), slot), slot);

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

  math_objects.free(slot);
  DynMathObject<type>& dyn_obj = add(slot);

  EqObject eq_obj {.equation = definition};

  auto assign_alternative = [&]<class T>(T&& obj) -> DynMathObject<type>&
  {
    math_objects.push(DynMathObject<type>(std::forward<T>(obj), slot), slot);
    return math_objects[slot];
  };

  auto ast = parsing::tokenize(definition)
               .and_then(parsing::make_ast{definition})
               .transform(parsing::flatten_separators);
  if (not ast)
    return assign_alternative(tl::unexpected(ast.error()));

  // the root node of the tree must be the equal sign
  if (not (ast->is_func() and ast->func_data().type == parsing::AST::Func::OP_ASSIGN))
    return assign_alternative(tl::unexpected(Error::not_math_object_definition()));


  auto& funcop_data = ast->func_data();

  assert(funcop_data.subnodes.size() == 2);

  eq_obj.lhs = std::move(funcop_data.subnodes[0]);
  eq_obj.rhs = std::move(funcop_data.subnodes[1]);

  funcop_data.subnodes.clear();

  // "f(x) = ...."
  const bool is_function_def = (eq_obj.lhs.is_func()
                                and eq_obj.lhs.func_data().type
                                      == parsing::AST::Func::FUNCTION);

  const bool is_sequence_def = is_function_def and eq_obj.rhs.is_func()
                               and eq_obj.rhs.func_data().type == parsing::AST::Func::SEPARATOR;

  // "var = complex expression that is not a number"
  const bool is_global_var_def = (eq_obj.lhs.is_var()
                                 and not eq_obj.rhs.is_number());

  // "var = 13.24213 (a number)"
  const bool is_global_constant_def = (eq_obj.lhs.is_var() and eq_obj.rhs.is_number());

  // first equation sanity check
  if (not is_function_def and not is_global_var_def and not is_global_constant_def) [[unlikely]]
    return assign_alternative(tl::unexpected(Error::unexpected(eq_obj.lhs.name, definition)));

  // second sanity check:
  // if the left side of the equation is a function call
  // it should only contain variables in each of its arguments, e.g. "f(x, y)"
  // and not e.g. "f(x^2 + 1, x + y)"
  if (is_function_def)
    for (const auto& arg: eq_obj.lhs.func_data().subnodes)
      if (not arg.is_var())
        return assign_alternative(tl::unexpected(Error::unexpected(arg.name, definition)));


  // third sanity check: type checks
  if constexpr (not std::is_same_v<InterpretAs, DynMathObject<type>>)
  {
    if constexpr (std::is_same_v<InterpretAs, Function<type>>)
    {
      if (is_sequence_def)
        // user asked for a function, but right hand side looks like a sequence def
        // cannot have Separators on the right hand side for functions
        return assign_alternative(
          tl::unexpected(Error::unexpected(eq_obj.rhs.name, definition)));

      if (is_global_var_def)
        return assign_alternative(
          tl::unexpected(Error::wrong_object_type(eq_obj.lhs.name, definition)));

      // better to have a too restricted assert than not
      assert(is_function_def and not is_sequence_def and not is_global_var_def
             and not is_global_constant_def);
    }
    else if constexpr (std::is_same_v<InterpretAs, Sequence<type>>)
    {
      if (is_global_var_def)
        return assign_alternative(
          tl::unexpected(Error::wrong_object_type(eq_obj.lhs.name, definition)));

      assert(is_function_def and is_sequence_def and not is_global_var_def
             and not is_global_constant_def);
    }
    else if constexpr (std::is_same_v<InterpretAs, GlobalConstant<type>>)
    {
      if (is_function_def)
        return assign_alternative(
          tl::unexpected(Error::wrong_object_type(eq_obj.lhs.name, definition)));
      if (is_global_var_def)
        return assign_alternative(
          tl::unexpected(Error::wrong_object_type(eq_obj.rhs.name, definition)));
      assert(is_global_constant_def and not is_global_var_def and not is_function_def
             and not is_sequence_def);
    }
    else static_assert(utils::dependent_false_v<InterpretAs>, "Possibility not covered");
  }

  // fourth sanity check: name check
  if (inventory.contains(eq_obj.lhs.name.substr))
    return assign_alternative(
      tl::unexpected(Error::name_already_taken(eq_obj.lhs.name, definition)));

  eq_obj.name = eq_obj.lhs.name.substr;

  std::vector<std::string> var_names;

  if (is_function_def)
  {
    // fill 'arg_names'
    const std::vector<parsing::AST>& args = eq_obj.lhs.func_data().subnodes;

    var_names.reserve(args.size());

    // the arguments of the function call in the left hand-side must all be regular variables
    for (const auto& arg: args)
      var_names.push_back(arg.name.substr);

    // mark function's input variables in 'rhs'
    eq_obj.rhs = parsing::mark_input_vars{var_names}(eq_obj.rhs);
  }

  // now that we checked that everything is fine, we can assign the object
  if (is_function_def or is_global_var_def)
  {
    if (is_sequence_def or zc::is_sequence_v<InterpretAs>)
      eq_obj.cat = EqObject::SEQUENCE;
    else
      eq_obj.cat = EqObject::FUNCTION;;
  }
  else
  {
    assert(is_global_constant_def);
    eq_obj.cat = EqObject::GLOBAL_CONSTANT;

    assign_alternative(GlobalConstant<type>(eq_obj.name, eq_obj.rhs.number_data().value));

    inventory[eq_obj.name] = slot;
  }

  dyn_obj.opt_eq_object = std::move(eq_obj);

  rebind_functions();

  return dyn_obj;
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
      assert(eq_obj.cat == EqObject::SEQUENCE or eq_obj.cat == EqObject ::FUNCTION);

      if (eq_obj.cat == EqObject::FUNCTION and not dyn_obj.template holds<Function<type>>())
        dyn_obj = Function<type>(eq_obj.name, eq_obj.lhs.args_num());
      else if (eq_obj.cat == EqObject::SEQUENCE and not dyn_obj.template holds<Function<type>>())
        dyn_obj = Sequence<type>(eq_obj.name);

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

  std::stack<std::string> invalid_functions;

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
            invalid_functions.push(eq_obj.name);
            dyn_obj = tl::unexpected(std::move(exp_rhs.error()));
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
              invalid_functions.push(eq_obj.name);
              dyn_obj = tl::unexpected(std::move(exp_rhs.error()));
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
    std::string invalid_func_name = invalid_functions.top();
    covered_invalid_functions.insert(invalid_func_name);
    invalid_functions.pop();

    if (auto it = inventory.find(invalid_func_name); it != inventory.end())
      inventory.erase(it);

    auto revdeps = direct_revdeps(invalid_func_name);
    for (auto&& [affected_func_name, info]: revdeps)
    {
      if(DynMathObject<type>* obj = get(affected_func_name))
      {
        // if these are revdeps, they can only be functions
        // because they have an expression that calls our invalid function
        assert(obj->template holds<Function<type>>() or obj->template holds<Sequence<type>>());

        assert(obj->opt_eq_object);

        *obj = tl::unexpected(
          Error::object_in_invalid_state(parsing::tokens::Text{.substr = invalid_func_name,
                                                               .begin = info.indexes.front()},
                                         obj->opt_eq_object->equation));

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

  inventory[name] = slot;

  obj = CppFunction<type, args_num>(name, cpp_f);
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
bool MathWorld<type>::sanity_check(const DynMathObject<type>& obj) const
{
  return math_objects.is_assigned(obj.slot) and &math_objects[obj.slot] == &obj;
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
deps::Deps MathWorld<type>::direct_dependencies(const DynMathObject<type>& obj) const
{
  if (not sanity_check(obj)) [[unlikely]]
    throw std::runtime_error("Object doesn't belong to math world");

  return direct_dependencies(obj.slot);
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

  if (std::string name = math_objects[slot].get_name(); not name.empty())
  {
    size_t erased_num = inventory.erase(name);
    assert(erased_num);
  }

  math_objects.free(slot);

  rebind_functions();

  return Ok{};
}

} // namespace zc
