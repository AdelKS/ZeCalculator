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
  return it != inventory.end() ? it->second : nullptr;
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
  size_t id = math_objects.next_free_slot();
  [[maybe_unused]] size_t new_id = math_objects.emplace(tl::unexpected(Error::empty_expression()), MathWorldObjectHandle<type>{id, this});
  assert(id == new_id); // should  be the same
  // TODO: fix this approach: calling next_free_slot then making sure it's the same

  return math_objects[id];
}

template <parsing::Type type>
template <class InterpretAs>
  requires (tuple_contains_v<MathEqObjects<type>, InterpretAs>
            or std::is_same_v<DynMathObject<type>, InterpretAs>)
DynMathObject<type>& MathWorld<type>::add(std::string definition)
{
 /**
  *  0. Example: def = "f(x) = cos(x)"
  *  1. Tokenize the definition: ['f', '(', 'x', ')', '=', 'cos', '(', 'x', ')']
  *  2. Make UAST representation of the definition
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

  auto& obj = add();

  MathEqObject<type> math_expr_obj(obj, definition);

  // sanity checks
  if (not sanity_check(obj))
  {
    obj = tl::unexpected(Error::object_not_in_world());
    return obj;
  }

  auto tokenization = parsing::tokenize(definition);
  if (not tokenization)
  {
    obj = tl::unexpected(tokenization.error());
    return obj;
  }

  tl::expected<parsing::UAST, Error> uast = parsing::make_uast(definition, *tokenization);
  if (not uast)
  {
    obj = tl::unexpected(uast.error());
    return obj;
  }

  // the root node of the tree must be the equal sign
  if (not std::holds_alternative<parsing::uast::node::Operator<'=', 2>>(**uast))
  {
    obj = tl::unexpected(Error::not_math_object_definition());
    return obj;
  }

  const auto& def_op = std::get<parsing::uast::node::Operator<'=', 2>>(**uast);

  math_expr_obj.lhs = def_op.operands[0];
  math_expr_obj.rhs = def_op.operands[1];

  // "f(x) = ...."
  const bool is_function_def = std::holds_alternative<parsing::uast::node::Function>(*math_expr_obj.lhs);

  // "var = complex expression that is not a number"
  const bool is_global_var_def = std::holds_alternative<parsing::uast::node::Variable>(*math_expr_obj.lhs)
                                 and not std::holds_alternative<parsing::shared::node::Number>(*math_expr_obj.rhs);

  // "var = 13.24213 (a number)"
  const bool is_global_constant_def = std::holds_alternative<parsing::uast::node::Variable>(*math_expr_obj.lhs)
                                 and std::holds_alternative<parsing::shared::node::Number>(*math_expr_obj.rhs);

  if (is_function_def or is_global_var_def)
  {
    if (is_global_var_def)
    {
      math_expr_obj.name = std::get<parsing::uast::node::Variable>(*math_expr_obj.lhs);
      obj = GlobalVariable<type>(math_expr_obj, {});
    }
    else // is_function_def
    {
      math_expr_obj.name = std::get<parsing::uast::node::Function>(*math_expr_obj.lhs).name_token;

      const std::vector<parsing::uast::node::NodePtr>& args = std::get<parsing::uast::node::Function>(*math_expr_obj.lhs).subnodes;

      // setting up the text that contains all the args
      parsing::tokens::Text args_txt;
      size_t first_arg_begin = parsing::text_token(*args.front()).substr_info->begin;
      size_t last_arg_begin = parsing::text_token(*args.back()).substr_info->begin;
      size_t last_arg_size = parsing::text_token(*args.back()).substr_info->size;
      args_txt.substr_info = SubstrInfo{.begin = first_arg_begin, .size = last_arg_begin - first_arg_begin + last_arg_size};
      args_txt.substr = args_txt.substr_info->substr(definition);

      // filling up the variables
      std::vector<parsing::uast::node::Variable> vars;
      for (const auto& arg: args)
      {
        // the arguments of the function call in the left hand-side must all be regular variables
        if (not std::holds_alternative<parsing::uast::node::Variable>(*arg))
        {
          obj = tl::unexpected(Error::unexpected(parsing::text_token(*arg), definition));
          return obj;
        }

        vars.push_back(std::get<parsing::uast::node::Variable>(*arg));
      }

      auto var_names = vars | std::views::transform(&parsing::tokens::Text::substr);

      // override 'rhs' with input variables of the function properly marked
      math_expr_obj.rhs = parsing::mark_input_vars{var_names}(math_expr_obj.rhs);

      // override 'obj' handle as a function
      bool valid_args_num = false;
      auto add_function = [&]<size_t args_num>(std::integral_constant<size_t, args_num>)
      {
        if (args_num != args.size())
          return;

        valid_args_num = true;
        obj = Function<type, args_num>(math_expr_obj, utils::to_array<args_num, parsing::tokens::Text>(vars));
      };
      utils::for_int_seq(add_function, std::make_index_sequence<max_func_args + 1>());

      if (not valid_args_num)
      {
        obj = tl::unexpected(Error::mismatched_fun_args(args_txt, definition));
        return obj;
      }

      if constexpr(zc::is_sequence_v<InterpretAs>)
      {
        if (vars.size() != 1)
        {
          obj = tl::unexpected(Error::mismatched_fun_args(args_txt, definition));
          return obj;
        }

        obj = Sequence(Function<type, 1>(math_expr_obj, std::array{parsing::tokens::Text(vars.front())}));
      }
    }
  }
  else if (is_global_constant_def)
  {
    math_expr_obj.name = std::get<parsing::uast::node::Variable>(*math_expr_obj.lhs);
    obj = GlobalConstant<type>(math_expr_obj, std::get<parsing::shared::node::Number>(*math_expr_obj.rhs));
  }
  else
  {
    obj = tl::unexpected(Error::unexpected(parsing::text_token(*math_expr_obj.lhs), definition));
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

  inventory[math_expr_obj.name.substr] = &obj;
  object_names[&obj] = math_expr_obj.name.substr;

  rebind_direct_revdeps_of(math_expr_obj.name.substr);

  return obj;
}

template <parsing::Type type>
template <size_t args_num>
  requires (args_num <= max_func_args)
DynMathObject<type>& MathWorld<type>::add(std::string name, CppMathFunctionPtr<args_num> cpp_f)
{
  auto& obj = add();

  if (inventory.contains(name))
  {
    obj = tl::unexpected(Error::name_already_taken(parsing::tokens::Text(name), ""));
    return obj;
  }

  object_names[&obj] = name;
  inventory[name] = &obj;

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

  size_t slot = obj.slot;
  erase(obj);
  auto& new_obj = add<InterpretAs>(definition);
  assert(new_obj.slot == slot);
  return new_obj;
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
void MathWorld<type>::rebind_direct_revdeps_of(const std::string& name)
{
  for (std::optional<DynMathObject<type>>& o: math_objects)
  {
    if (o and o->has_value())
      std::visit(
        [&]<class T>(T& obj) {
          if constexpr (is_function_v<T>)
            if (obj.direct_dependencies().contains(name))
              obj.rebind();
        },
        **o);
  }
}

template <parsing::Type type>
tl::expected<double, Error> MathWorld<type>::evaluate(std::string expr)
{
  if (expr.empty()) [[unlikely]]
    return tl::unexpected(Error::empty_expression());

  auto make_uast = [&](std::span<const parsing::Token> tokens)
  {
    return parsing::make_uast(expr, tokens);
  };

  auto evaluate = [](const parsing::Parsing<type>& repr)
  {
    return zc::evaluate(repr);
  };

  if constexpr (type == parsing::Type::AST)
    return parsing::tokenize(expr)
      .and_then(make_uast)
      .and_then(parsing::bind<type>{expr, *this})
      .and_then(evaluate);
  else
    return parsing::tokenize(expr)
      .and_then(make_uast)
      .and_then(parsing::bind<type>{expr, *this})
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

  return erase(math_objects[obj.slot]);
}

template <parsing::Type type>
tl::expected<Ok, UnregisteredObject> MathWorld<type>::erase(DynMathObject<type>& obj)
{
  size_t slot = obj.slot;

  if (not sanity_check(obj))
    return tl::unexpected(UnregisteredObject{});

  const auto name_node = object_names.extract(&math_objects[slot]);
  if (bool(name_node))
  {
    // extract "name" from the inventory and just throw it away
    auto node = inventory.extract(name_node.mapped());

    // the inventory *should* have a valid node
    assert(bool(node));

    // functions could be depending on 'name' so re-parse them
    rebind_direct_revdeps_of(name_node.mapped());
  }

  // remove math object
  math_objects.pop(slot);

  return Ok{};
}

template <parsing::Type type>
tl::expected<Ok, UnregisteredObject> MathWorld<type>::erase(const std::string& name)
{
  DynMathObject<type>* dyn_obj = get(name);
  if (not dyn_obj)
    return tl::unexpected(UnregisteredObject{});
  else return erase(*dyn_obj);
}


} // namespace zc
