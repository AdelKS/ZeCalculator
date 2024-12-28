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
#include <zecalculator/math_objects/impl/data.h>
#include <zecalculator/math_objects/impl/function.h>
#include <zecalculator/math_objects/impl/global_constant.h>
#include <zecalculator/math_objects/impl/internal/eq_object.h>
#include <zecalculator/math_objects/impl/sequence.h>
#include <zecalculator/parsing/impl/utils.h>
#include <zecalculator/utils/utils.h>

namespace zc {

template <parsing::Type type>
DynMathObject<type>::DynMathObject(
  tl::expected<MathObjectsVariant<type>, Error> exp_variant,
  size_t slot,
  MathWorld<type>& mathworld)
  : tl::expected<MathObjectsVariant<type>, Error>(std::move(exp_variant)), slot(slot),
    mathworld(mathworld)
{}

template <parsing::Type type>
tl::expected<double, Error> DynMathObject<type>::operator () (std::initializer_list<double> vals, eval::Cache* cache) const
{
  return evaluate(vals, cache);
}

template <parsing::Type type>
tl::expected<double, Error> DynMathObject<type>::evaluate(std::initializer_list<double> vals, eval::Cache* cache) const
{
  using Ret = tl::expected<double, Error>;
  if (not bool(*this))
    return tl::unexpected(this->error());

  return std::visit(
    utils::overloaded{
      [&]<size_t args_num>(const CppFunction<args_num>& cpp_f) -> Ret
      {
        if (vals.size() != args_num)
          return tl::unexpected(Error::cpp_incorrect_argnum());

        return cpp_f(std::span<const double, args_num>(vals.begin(), args_num));
      },
      [&](const Function<type>& f) -> Ret
      {
        // argument size test done within Function's code
        return f(vals, cache);
      },
      [&](const GlobalConstant& cst) -> Ret
      {
        if (vals.size() != 0)
          return tl::unexpected(Error::cpp_incorrect_argnum());
        else return cst.value;
      },
      [&](const Sequence<type>& u) -> Ret
      {
        if (vals.size() != 1)
          return tl::unexpected(Error::cpp_incorrect_argnum());
        else return u(*vals.begin(), cache);
      },
      [&](const Data<type>& d) -> Ret
      {
        if (vals.size() != 1)
          return tl::unexpected(Error::cpp_incorrect_argnum());
        else return d(*vals.begin(), cache);
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
    return assign(std::move(eq.str), internal::EqObject::FUNCTION);
  else if constexpr (std::is_same_v<T, Sequence<type>>)
    return assign(std::move(eq.str), internal::EqObject::SEQUENCE);
  else if constexpr (std::is_same_v<T, GlobalConstant>)
    return assign(std::move(eq.str), internal::EqObject::GLOBAL_CONSTANT);
  else if constexpr (std::is_same_v<T, Data<type>>)
    return assign(std::move(eq));
  else static_assert(utils::dependent_false_v<T>, "case not handled");
}

template <parsing::Type type>
DynMathObject<type>& DynMathObject<type>::operator = (std::string eq)
{
  return assign(std::move(eq), internal::EqObject::AUTO);
}

template <parsing::Type type>
template <size_t args_num>
DynMathObject<type>& DynMathObject<type>::operator = (CppFunction<args_num> cpp_f)
{
  auto exp_lhs = parsing::parse_lhs(cpp_f.get_name(), cpp_f.get_name());
  if (not exp_lhs) [[unlikely]]
    return assign_error(exp_lhs.error());
  else if(not exp_lhs->input_vars.empty()) [[unlikely]]
    return assign_error(zc::Error::unexpected(exp_lhs->input_vars.front(), cpp_f.name));
  else if (mathworld.contains(exp_lhs->name.substr)) [[unlikely]]
    return assign_error(Error::name_already_taken(exp_lhs->name.substr));
  else
  {
    cpp_f.name = exp_lhs->name.substr;
    return assign_object(std::move(cpp_f), {});
  }
}

template <parsing::Type type>
DynMathObject<type>& DynMathObject<type>::operator = (GlobalConstant cst)
{
  auto exp_lhs = parsing::parse_lhs(cst.get_name(), cst.get_name());
  if (not exp_lhs) [[unlikely]]
    return assign_error(exp_lhs.error());
  else if(not exp_lhs->input_vars.empty()) [[unlikely]]
    return assign_error(zc::Error::unexpected(exp_lhs->input_vars.front(), cst.name));
  else if (mathworld.contains(exp_lhs->name.substr)) [[unlikely]]
    return assign_error(Error::name_already_taken(exp_lhs->name.substr));
  else
  {
    cst.name = exp_lhs->name.substr;
    return assign_object(std::move(cst), {});
  }
}

template <parsing::Type type>
tl::expected<MathObjectsVariant<type>, Error>& DynMathObject<type>::as_expected()
{
  return *this;
}

template <parsing::Type type>
const tl::expected<MathObjectsVariant<type>, Error>&
  DynMathObject<type>::as_expected() const
{
  return *this;
}

template <parsing::Type type>
DynMathObject<type>& DynMathObject<type>::assign(std::string definition, internal::EqObject::Category cat)
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

  internal::EqObject eq_obj {.cat = cat, .equation = definition};

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

  eq_obj.rhs = std::move(funcop_data.subnodes[1]);

  using parsing::LHS, parsing::parse_lhs;

  tl::expected<LHS, zc::Error> exp_parsed_lhs = parse_lhs(funcop_data.subnodes[0], definition);
  if (not bool(exp_parsed_lhs))
    return assign_error(exp_parsed_lhs.error());

  const LHS& parsed_lhs = *exp_parsed_lhs;

  eq_obj.lhs = parsed_lhs.substr;

  // "f(x) = ...."
  const bool is_function_def = not parsed_lhs.input_vars.empty();

  const bool is_sequence_def = is_function_def and eq_obj.rhs.is_func()
                               and eq_obj.rhs.func_data().type == parsing::AST::Func::SEPARATOR;

  // "var = complex expression that is not a number"
  const bool is_global_var_def = (parsed_lhs.input_vars.empty() and not eq_obj.rhs.is_number());

  // "var = 13.24213 (a number)"
  const bool is_global_constant_def = (parsed_lhs.input_vars.empty() and eq_obj.rhs.is_number());

  // first equation sanity check
  if (not is_function_def
      and not is_global_var_def
      and not is_sequence_def
      and not is_global_constant_def) [[unlikely]]
    return assign_error(Error::unexpected(parsed_lhs.name, definition));

  // third sanity check: type checks
  if (cat != internal::EqObject::AUTO)
  {
    eq_obj.cat = cat;

    if (cat == internal::EqObject::FUNCTION)
    {
      if (is_sequence_def)
        // user asked for a function, but right hand side looks like a sequence def
        // cannot have Separators on the right hand side for functions
        return assign_error(Error::unexpected(eq_obj.rhs.name, definition));

      // a function def, global var def and global constant def can all be considered
      // functions
      assert((is_function_def or is_global_var_def or is_global_constant_def)
             and not is_sequence_def);
    }
    else if (cat == internal::EqObject::SEQUENCE)
    {
      // global vars and constants cannot be considered sequences
      // due to the strict requirement that sequences have are single
      // argument functions. Whereas vars and constants are zero argument functions
      if (is_global_var_def or is_global_constant_def)
        return assign_error(Error::wrong_object_type(parsed_lhs.name, definition));

      assert(is_function_def and not is_global_var_def
             and not is_global_constant_def);
    }
    else if (cat == internal::EqObject::GLOBAL_CONSTANT)
    {
      if (is_function_def)
        return assign_error(Error::wrong_object_type(parsed_lhs.name, definition));
      if (is_global_var_def)
        return assign_error(Error::wrong_object_type(eq_obj.rhs.name, definition));
      assert(is_global_constant_def and not is_global_var_def and not is_function_def
             and not is_sequence_def);
    }
    else [[unlikely]] assert(false);
  }

  eq_obj.name = parsed_lhs.name;

  std::string old_name(get_name());

  // fourth sanity check: name check
  if (eq_obj.name.substr != old_name and mathworld.contains(eq_obj.name.substr))
    return assign_error(Error::name_already_taken(parsed_lhs.name, definition));

  eq_obj.var_names.reserve(parsed_lhs.input_vars.size());
  std::ranges::transform(parsed_lhs.input_vars,
                         std::back_inserter(eq_obj.var_names),
                         &parsing::tokens::Text::substr);

  // now that we checked that everything is fine, we can assign the object
  if (cat == internal::EqObject::Category::AUTO)
  {
    if (is_sequence_def or cat == internal::EqObject::SEQUENCE)
      eq_obj.cat = internal::EqObject::SEQUENCE;

    else if (is_function_def or is_global_var_def)
      eq_obj.cat = internal::EqObject::FUNCTION;

    else
    {
      assert(is_global_constant_def);
      eq_obj.cat = internal::EqObject::GLOBAL_CONSTANT;
    }
  }

  assign_object(eq_obj.to_expected(slot, mathworld), eq_obj);

  return *this;
}

template <parsing::Type type>
DynMathObject<type>& DynMathObject<type>::assign(As<Data<type>> data_def)
{
  using parsing::LHS, parsing::parse_lhs;

  tl::expected<LHS, zc::Error> exp_parsed_lhs = parse_lhs(data_def.func_name, data_def.func_name);
  if (not bool(exp_parsed_lhs))
    return assign_error(exp_parsed_lhs.error());

  const LHS& parsed_lhs = *exp_parsed_lhs;

  if (mathworld.contains(parsed_lhs.name.substr)) [[unlikely]]
    return assign_error(Error::name_already_taken(parsed_lhs.name.substr));

  if(parsed_lhs.input_vars.size() > 1)
    return assign_error(Error::unexpected(parsed_lhs.input_vars[1], data_def.func_name));

  return assign_object(Data<type>(slot,
                                  std::move(parsed_lhs.name.substr),
                                  parsed_lhs.input_vars.empty()
                                    ? std::move(data_def.default_index_var_name)
                                    : std::move(parsed_lhs.input_vars.front().substr),
                                  std::move(data_def.str_data),
                                  &mathworld));
}

template <parsing::Type type>
DynMathObject<type>&
  DynMathObject<type>::assign_error(Error error, std::optional<internal::EqObject> new_opt_eq_obj)
{
  return assign_object(tl::unexpected(std::move(error)), std::move(new_opt_eq_obj));
}

template <parsing::Type type>
template <class T>
DynMathObject<type>&
  DynMathObject<type>::assign_object(T&& obj, std::optional<internal::EqObject> new_opt_eq_obj)
{
  std::string old_name(get_name());
  auto old_eq_object = opt_eq_object;
  opt_eq_object = std::move(new_opt_eq_obj);
  as_expected() = std::forward<T>(obj);

  mathworld.object_updated(slot,
                           bool(opt_eq_object),
                           old_eq_object ? old_eq_object->name.substr : old_name,
                           opt_eq_object ? opt_eq_object->name.substr : std::string(get_name()));

  return *this;
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
  if (opt_eq_object)
    return opt_eq_object->name.substr;
  else if (bool(*this))
    return std::visit([](const auto& val){ return val.get_name(); }, **this);
  else return std::string_view();
}

template <parsing::Type type>
bool DynMathObject<type>::has_function_eq_obj() const
{
  return opt_eq_object
         and (opt_eq_object->cat == internal::EqObject::FUNCTION or opt_eq_object->cat == internal::EqObject::SEQUENCE);
}

template <parsing::Type type>
deps::Deps DynMathObject<type>::direct_dependencies() const
{
  if (not opt_eq_object)
    return deps::Deps();

  return opt_eq_object->direct_dependencies();
}

template <parsing::Type type>
std::optional<std::string> DynMathObject<type>::get_equation() const
{
  if (opt_eq_object)
    return opt_eq_object->equation;
  else if (not this->has_value())
  {
    const Error& err = this->error();
    if (not err.expression.empty())
      return err.expression;
  }
  return {};
}

}
