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

#include <zecalculator/math_objects/decl/dyn_math_object.h>
#include <zecalculator/math_objects/impl/cpp_function.h>
#include <zecalculator/math_objects/impl/function.h>
#include <zecalculator/math_objects/impl/global_constant.h>
#include <zecalculator/math_objects/impl/sequence.h>
#include <zecalculator/utils/utils.h>

namespace zc {

template <parsing::Type type>
DynMathObject<type>::DynMathObject(tl::expected<MathObjectsVariant<type>, Error> exp_variant, size_t slot, MathWorld<type>& mathworld)
  : tl::expected<MathObjectsVariant<type>, Error>(std::move(exp_variant)), slot(slot), mathworld(mathworld)
{}

template <parsing::Type type>
template <class... DBL>
  requires (std::is_convertible_v<DBL, double> and ...)
tl::expected<double, Error> DynMathObject<type>::evaluate(DBL... val) const
{
  using Ret = tl::expected<double, Error>;
  if (not bool(*this))
    return tl::unexpected(this->error());

  return std::visit(
    utils::overloaded{
      [&]<size_t args_num>(const CppFunction<args_num>& cpp_f) -> Ret
      {
        if constexpr (sizeof...(val) != args_num)
          return tl::unexpected(Error::cpp_incorrect_argnum());
        else return cpp_f(val...);
      },
      [&](const Function<type>& f) -> Ret
      {
        // argument size test done within Function's code
        return f(val...);
      },
      [&](const GlobalConstant& cst) -> Ret
      {
        if constexpr (sizeof...(val) != 0)
          return tl::unexpected(Error::cpp_incorrect_argnum());
        else return cst.value;
      },
      [&](const Sequence<type>& u) -> Ret
      {
        if constexpr (sizeof...(val) != 1)
          return tl::unexpected(Error::cpp_incorrect_argnum());
        else return u(val...);
      }
    },
    **this
  );
}

template <parsing::Type type>
template <class T>
  requires tuple_contains_v<MathEqObjects<type>, T>
DynMathObject<type>& DynMathObject<type>::operator = (As<T> eq)
{
  if constexpr (std::is_same_v<T, Function<type>>)
    return assign(std::move(eq.str), EqObject::FUNCTION);
  else if constexpr (std::is_same_v<T, Sequence<type>>)
    return assign(std::move(eq.str), EqObject::SEQUENCE);
  else if constexpr (std::is_same_v<T, GlobalConstant>)
    return assign(std::move(eq.str), EqObject::GLOBAL_CONSTANT);
  else static_assert(utils::dependent_false_v<T>, "case not handled");
}

template <parsing::Type type>
DynMathObject<type>& DynMathObject<type>::operator = (std::string eq)
{
  return assign(std::move(eq), EqObject::AUTO);
}

template <parsing::Type type>
template <size_t args_num>
DynMathObject<type>& DynMathObject<type>::operator = (CppFunction<args_num> cpp_f)
{
  opt_eq_object.reset();

  if (mathworld.contains(cpp_f.get_name())) [[unlikely]]
    return assign_error(Error::name_already_taken(std::string(cpp_f.get_name())));
  else return assign_object(std::move(cpp_f), {});
}

template <parsing::Type type>
DynMathObject<type>& DynMathObject<type>::operator = (GlobalConstant cst)
{
  opt_eq_object.reset();

  if (mathworld.contains(cst.get_name())) [[unlikely]]
    return assign_error(Error::name_already_taken(std::string(cst.get_name())));
  else return assign_object(std::move(cst), {});

  return *this;
}

template <parsing::Type type>
tl::expected<MathObjectsVariant<type>, Error>& DynMathObject<type>::as_expected()
{
  return *this;
}

template <parsing::Type type>
const tl::expected<MathObjectsVariant<type>, Error>& DynMathObject<type>::as_expected() const
{
  return *this;
}

template <parsing::Type type>
DynMathObject<type>& DynMathObject<type>::assign(std::string definition, EqObject::Category cat)
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

  EqObject eq_obj {.cat = cat, .equation = definition};

  auto ast = parsing::tokenize(definition)
               .and_then(parsing::make_ast{definition})
               .transform(parsing::flatten_separators);
  if (not ast)
    return assign_error(ast.error());

  // the root node of the tree must be the equal sign
  if (not (ast->is_func() and ast->func_data().type == parsing::AST::Func::OP_ASSIGN))
    return assign_error(Error::not_math_object_definition());


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
    return assign_error(Error::unexpected(eq_obj.lhs.name, definition));

  // second sanity check:
  // if the left side of the equation is a function call
  // it should only contain variables in each of its arguments, e.g. "f(x, y)"
  // and not e.g. "f(x^2 + 1, x + y)"
  if (is_function_def)
    for (const auto& arg: eq_obj.lhs.func_data().subnodes)
      if (not arg.is_var())
        return assign_error(Error::unexpected(arg.name, definition));


  // third sanity check: type checks
  if (cat != EqObject::AUTO)
  {
    if (cat == EqObject::FUNCTION)
    {
      if (is_sequence_def)
        // user asked for a function, but right hand side looks like a sequence def
        // cannot have Separators on the right hand side for functions
        return assign_error(Error::unexpected(eq_obj.rhs.name, definition));

      if (is_global_var_def)
        return assign_error(Error::wrong_object_type(eq_obj.lhs.name, definition));

      // better to have a too restricted assert than not
      assert(is_function_def and not is_sequence_def and not is_global_var_def
             and not is_global_constant_def);
    }
    else if (cat == EqObject::SEQUENCE)
    {
      if (is_global_var_def or is_global_constant_def)
        return assign_error(Error::wrong_object_type(eq_obj.lhs.name, definition));

      assert(is_function_def and not is_global_var_def
             and not is_global_constant_def);
    }
    else if (cat == EqObject::GLOBAL_CONSTANT)
    {
      if (is_function_def)
        return assign_error(Error::wrong_object_type(eq_obj.lhs.name, definition));
      if (is_global_var_def)
        return assign_error(Error::wrong_object_type(eq_obj.rhs.name, definition));
      assert(is_global_constant_def and not is_global_var_def and not is_function_def
             and not is_sequence_def);
    }
    else [[unlikely]] assert(false);
  }

  eq_obj.name = eq_obj.lhs.name.substr;

  std::string old_name(get_name());

  // fourth sanity check: name check
  if (eq_obj.name != old_name and mathworld.contains(eq_obj.name))
    return assign_error(Error::name_already_taken(eq_obj.lhs.name, definition));

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
  if (is_sequence_def or cat == EqObject::SEQUENCE)
    eq_obj.cat = EqObject::SEQUENCE;

  else if (is_function_def or is_global_var_def)
    eq_obj.cat = EqObject::FUNCTION;

  else
  {
    assert(is_global_constant_def);
    eq_obj.cat = EqObject::GLOBAL_CONSTANT;
  }

  assign_object(eq_obj.to_expected(mathworld), eq_obj);

  return *this;
}

template <parsing::Type type>
DynMathObject<type>& DynMathObject<type>::assign_error(Error error, std::optional<EqObject> new_opt_eq_obj)
{
  return assign_object(tl::unexpected(std::move(error)), std::move(new_opt_eq_obj));
}

template <parsing::Type type>
template <class T>
DynMathObject<type>& DynMathObject<type>::assign_object(T&& obj, std::optional<EqObject> new_opt_eq_obj)
{
  std::string old_name(get_name());
  auto old_eq_object = opt_eq_object;
  opt_eq_object = std::move(new_opt_eq_obj);
  as_expected() = std::forward<T>(obj);

  mathworld.object_updated(slot,
                           bool(opt_eq_object),
                           old_eq_object ? old_eq_object->name : old_name,
                           opt_eq_object ? opt_eq_object->name : std::string(get_name()));

  return *this;
}

template <parsing::Type type>
template <class... DBL>
  requires (std::is_convertible_v<DBL, double> and ...)
tl::expected<double, Error> DynMathObject<type>::operator () (DBL... val) const
{
  return evaluate(val...);
}

template <parsing::Type type>
template <class T>
  requires (tuple_contains_v<MathObjects<type>, T>)
const T& DynMathObject<type>::value_as() const
{
  return std::get<T>(this->value());
}


template <parsing::Type type>
template <class T>
  requires (tuple_contains_v<MathObjects<type>, T>)
T& DynMathObject<type>::value_as()
{
  return std::get<T>(this->value());
}

template <parsing::Type type>
template <class T>
  requires (tuple_contains_v<MathObjects<type>, T> or std::is_same_v<T, Error>)
bool DynMathObject<type>::holds() const
{
  if constexpr (std::is_same_v<T, Error>)
    return not this->has_value();
  else
  {
    if (this->has_value())
      return std::holds_alternative<T>(**this);
    else return false;
  }
}

template <parsing::Type type>
std::string_view DynMathObject<type>::get_name() const
{
  if (bool(*this))
    return std::visit([](const auto& val){ return val.get_name(); }, **this);
  else return std::string_view();
}

template <parsing::Type type>
bool DynMathObject<type>::has_function_eq_obj() const
{
  return opt_eq_object
         and (opt_eq_object->cat == EqObject::FUNCTION or opt_eq_object->cat == EqObject::SEQUENCE);
}

template <parsing::Type type>
deps::Deps DynMathObject<type>::direct_dependencies() const
{
  if (not opt_eq_object)
    return deps::Deps();

  return parsing::direct_dependencies(opt_eq_object->rhs);
}

}
