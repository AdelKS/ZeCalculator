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
template <size_t args_num>
DynMathObject<type>& DynMathObject<type>::set(std::string name, CppFunction<args_num> cpp_f)
{
  std::string old_name(get_name());
  opt_equation.reset();
  obj_type = CPP_FUNCTION;

  parsed_data = cpp_f;
  exp_lhs = parsing::parse_lhs(name, name);

  if (get_name() != old_name and mathworld.contains(get_name()))
  {
    std::string full_expr = opt_equation ? *opt_equation : exp_lhs->substr.substr;
    exp_lhs = tl::unexpected(Error::name_already_taken(exp_lhs->name, full_expr));
  }
  assign_alternative();

  mathworld.object_updated(slot, false, old_name, std::string(get_name()));

  return *this;
}

template <parsing::Type type>
DynMathObject<type>& DynMathObject<type>::set(std::string name, GlobalConstant cst)
{
  std::string old_name(get_name());
  opt_equation.reset();
  obj_type = CONSTANT;

  parsed_data = cst.value;
  exp_lhs = parsing::parse_lhs(name, name);

  if (get_name() != old_name and mathworld.contains(get_name()))
  {
    std::string full_expr = opt_equation ? *opt_equation : exp_lhs->substr.substr;
    exp_lhs = tl::unexpected(Error::name_already_taken(exp_lhs->name, full_expr));
  }
  assign_alternative();

  mathworld.object_updated(slot, false, old_name, std::string(get_name()));

  return *this;
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
DynMathObject<type>& DynMathObject<type>::operator = (std::string definition)
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

  std::string old_name(get_name());

  obj_type = BAD_EQUATION;
  exp_lhs = tl::unexpected(zc::Error::unkown());
  opt_equation = definition;

  auto ast = parsing::tokenize(definition)
               .and_then(parsing::make_ast{definition})
               .transform(parsing::flatten_separators);
  if (not ast)
  {
    parsed_data = ast.error();
  }

  // the root node of the tree must be the equal sign
  else if (not (ast->is_func() and ast->func_data().type == parsing::AST::Func::OP_ASSIGN))
  {
    parsed_data = Error::not_math_object_definition();
  }
  else
  {
    auto& funcop_data = ast->func_data();

    assert(funcop_data.subnodes.size() == 2);

    auto& rhs = funcop_data.subnodes[1];

    if (rhs.is_func() and rhs.func_data().type == parsing::AST::Func::SEPARATOR)
    {
      obj_type = SEQUENCE;
      parsed_data = SeqObj{.rhs = std::move(rhs.func_data().subnodes)};
    }
    else if (rhs.is_number())
    {
      obj_type = CONSTANT;
      parsed_data = rhs.number_data().value;
    }
    else
    {
      obj_type = FUNCTION;
      parsed_data = FuncObj{.rhs = std::move(rhs)};
    }

    exp_lhs = parsing::parse_lhs(funcop_data.subnodes[0], definition);
  }

  if (get_name() != old_name and mathworld.contains(get_name()))
  {
    std::string full_expr = opt_equation ? *opt_equation : exp_lhs->substr.substr;
    exp_lhs = tl::unexpected(Error::name_already_taken(exp_lhs->name, full_expr));
  }
  assign_alternative();

  mathworld.object_updated(slot,
                           std::holds_alternative<SeqObj>(parsed_data)
                             or std::holds_alternative<FuncObj>(parsed_data),
                           old_name,
                           std::string(get_name()));

  return *this;
}

template <parsing::Type type>
DynMathObject<type>& DynMathObject<type>::set_data(std::string name, std::vector<std::string> data)
{
  std::string old_name(get_name());
  opt_equation.reset();
  obj_type = DATA;

  parsed_data = DataObj{.data = std::move(data)};
  exp_lhs = parsing::parse_lhs(name, name);

  if (get_name() != old_name and mathworld.contains(get_name()))
  {
    std::string full_expr = opt_equation ? *opt_equation : exp_lhs->substr.substr;
    exp_lhs = tl::unexpected(Error::name_already_taken(exp_lhs->name, full_expr));
  }
  assign_alternative();

  mathworld.object_updated(slot,
                           false,
                           old_name,
                           std::string(get_name()));

  return *this;
}

template <parsing::Type type>
template <bool linked>
DynMathObject<type>& DynMathObject<type>::assign_alternative()
{
  if (not bool(exp_lhs))
  {
    as_expected() = tl::unexpected(exp_lhs.error());
    return *this;
  }

  if(obj_type == DATA and exp_lhs->input_vars.size() > 1)
  {
    as_expected() = tl::unexpected(Error::unexpected(exp_lhs->input_vars[1], exp_lhs->substr.substr));
    return *this;
  }

  auto get_final_representation = [&](parsing::AST ast)
  {
    std::string equation;
    if (opt_equation)
      equation = *opt_equation;

    auto var_names = exp_lhs->input_vars | std::views::transform(&parsing::tokens::Text::substr);
    auto final_ast = parsing::mark_input_vars{var_names}(ast);
    if constexpr (type == parsing::Type::FAST)
      return parsing::make_fast<type>{equation, mathworld}(final_ast);
    else
      return parsing::make_fast<type>{equation, mathworld}(final_ast).transform(parsing::make_RPN);
  };

  std::visit(
    utils::overloaded{
      [&](zc::Error err)
      {
        as_expected() = tl::unexpected(err);
      },
      [&](double val)
      {
        as_expected() = GlobalConstant{val};
      },
      [&]<size_t args_num>(CppFunction<args_num> f)
      {
        as_expected() = f;
      },
      [&](const FuncObj& f_obj)
      {
        if constexpr (linked)
        {
          auto exp_final_repr = get_final_representation(f_obj.rhs);
          if (exp_final_repr)
            as_expected() = Function<type>(exp_lhs->input_vars.size(),
                                          *exp_final_repr);
          else
            as_expected() = tl::unexpected(exp_final_repr.error());
        }
        else as_expected() = Function<type>(exp_lhs->input_vars.size());
      },
      [&](const SeqObj& seq_obj)
      {
        if constexpr (linked)
        {
          std::vector<parsing::Parsing<type>> values;
          values.reserve(seq_obj.rhs.size());
          for (const parsing::AST& ast : seq_obj.rhs)
            if (auto exp_rhs = get_final_representation(ast))
              values.push_back(std::move(*exp_rhs));
            else
            {
              as_expected() = tl::unexpected(std::move(exp_rhs.error()));
              return;
            }
          as_expected() = Sequence<type>(slot, std::move(values));
        }
        else as_expected() = Sequence<type>();
      },
      [&](const DataObj& data_obj)
      {
        as_expected() = Data<type>(slot,
                                  exp_lhs->name.substr,
                                  not exp_lhs->input_vars.empty()
                                    ? exp_lhs->input_vars.front().substr
                                    : "",
                                  std::move(data_obj.data),
                                  &mathworld);
      }
    }, parsed_data);

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
  if (exp_lhs)
    return exp_lhs->name.substr;
  else return std::string_view();
}

template <parsing::Type type>
std::vector<std::string> DynMathObject<type>::get_input_var_names() const
{
  std::vector<std::string> var_names;
  if (exp_lhs)
  {
    var_names.reserve(exp_lhs->input_vars.size());
    std::ranges::transform(exp_lhs->input_vars,
                           std::back_inserter(var_names),
                           &parsing::tokens::Text::substr);
  }
  return var_names;
}

template <parsing::Type type>
std::optional<std::string> DynMathObject<type>::get_equation() const
{
  return opt_equation;
}

template <parsing::Type type>
deps::Deps DynMathObject<type>::direct_dependencies() const
{
  return std::visit(
    utils::overloaded{
      [&](zc::Error)
      {
        return deps::Deps();
      },
      [&](double)
      {
        return deps::Deps();
      },
      [&]<size_t args_num>(CppFunction<args_num>)
      {
        return deps::Deps();
      },
      [&](const FuncObj& f_obj)
      {
        auto var_names = exp_lhs->input_vars | std::views::transform(&parsing::tokens::Text::substr);
        return parsing::direct_dependencies(parsing::mark_input_vars{var_names}(f_obj.rhs));
      },
      [&](const SeqObj& seq_obj)
      {
        auto deps = deps::Deps();
        auto var_names = exp_lhs->input_vars | std::views::transform(&parsing::tokens::Text::substr);
        for (const parsing::AST& ast: seq_obj.rhs)
        {
          auto extra_deps = parsing::direct_dependencies(parsing::mark_input_vars{var_names}(ast));
          deps.insert(extra_deps.begin(), extra_deps.end());
        }
        return deps;
      },
      [&](const DataObj&)
      {
        return deps::Deps();
      }
    }, parsed_data);
}

}
