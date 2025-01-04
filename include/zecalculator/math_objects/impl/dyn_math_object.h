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

#include <zecalculator/evaluation/impl/evaluation.h>
#include <zecalculator/math_objects/decl/dyn_math_object.h>
#include <zecalculator/math_objects/impl/cpp_function.h>
#include <zecalculator/parsing/impl/utils.h>
#include <zecalculator/utils/utils.h>

namespace zc {

template <parsing::Type type>
tl::expected<double, Error> DynMathObject<type>::operator () (std::initializer_list<double> vals, eval::Cache* cache) const
{
  return evaluate(vals, cache);
}

template <parsing::Type type>
DynMathObject<type>& DynMathObject<type>::set_name(std::string_view name)
{
  std::string old_name(get_name());

  set_name_internal(name, name);

  finalize_asts();

  mathworld.object_updated(slot, false, old_name, std::string(get_name()));

  return *this;
}

template <parsing::Type type>
template <class T>
  requires (std::is_same_v<T, parsing::AST> or std::is_convertible_v<T, std::string_view>)
void DynMathObject<type>::set_name_internal(const T& name, std::string_view full_expr)
{
  std::string old_name(get_name());

  lhs_str = std::string(full_expr);
  exp_lhs = parsing::parse_lhs(name, full_expr);

  if (get_name() != old_name and mathworld.contains(get_name()))
    exp_lhs = tl::unexpected(Error::name_already_taken(exp_lhs->name, std::string(full_expr)));

  if (bool(exp_lhs))
  {
    if ((holds(SEQUENCE) or holds(DATA)) and exp_lhs->input_vars.size() > 1)
      exp_lhs = tl::unexpected(
        zc::Error::unexpected(exp_lhs->input_vars[1], std::string(full_expr)));
    else if ((holds(CONSTANT) or holds(CPP_FUNCTION)) and not exp_lhs->input_vars.empty())
      exp_lhs = tl::unexpected(
        zc::Error::unexpected(exp_lhs->input_vars[0], std::string(full_expr)));
  }
}

template <parsing::Type type>
tl::expected<double, Error> DynMathObject<type>::evaluate(std::initializer_list<double> vals, eval::Cache* cache) const
{
  using Ret = tl::expected<double, Error>;
  if (auto err = error())
    return tl::unexpected(*err);

  return std::visit(
    utils::overloaded{
      [&](zc::Error err) -> Ret
      {
        return tl::unexpected(err);
      },
      [&]<size_t args_num>(CppFunction<args_num> cpp_f) -> Ret
      {
        if (vals.size() != args_num)
          return tl::unexpected(Error::cpp_incorrect_argnum());

        return cpp_f(std::span<const double, args_num>(vals.begin(), args_num));
      },
      [&](const FuncObj& f_obj) -> Ret
      {
        if (not bool(f_obj.linked_rhs))
          return tl::unexpected(f_obj.linked_rhs.error());
        else if (f_obj.linked_rhs->args_num != vals.size())
          return tl::unexpected(zc::Error::cpp_incorrect_argnum());
        return zc::evaluate(f_obj.linked_rhs->repr, vals, cache);
      },
      [&](const ConstObj& cst) -> Ret
      {
        if (vals.size() != 0)
          return tl::unexpected(Error::cpp_incorrect_argnum());
        else return cst.val;
      },
      [&](const SeqObj& seq_obj) -> Ret
      {
        if (vals.size() != 1)
          return tl::unexpected(Error::cpp_incorrect_argnum());
        else if (not bool(seq_obj.linked_rhs))
          return tl::unexpected(seq_obj.linked_rhs.error());
        else return zc::evaluate(*seq_obj.linked_rhs, *vals.begin(), cache);
      },
      [&](const DataObj& data_obj) -> Ret
      {
        if (vals.size() != 1)
          return tl::unexpected(Error::cpp_incorrect_argnum());
        else return zc::evaluate(data_obj.linked_rhs, *vals.begin(), cache);
      }
    },
    parsed_data
  );
}

template <parsing::Type type>
template <size_t args_num>
DynMathObject<type>& DynMathObject<type>::set(std::string_view name, CppFunction<args_num> cpp_f)
{
  std::string old_name(get_name());

  parsed_data = cpp_f;

  set_name_internal(name, name);

  mathworld.object_updated(slot, false, old_name, std::string(get_name()));

  return *this;
}

template <parsing::Type type>
template <size_t args_num>
DynMathObject<type>& DynMathObject<type>::operator = (CppFunction<args_num> cpp_f)
{
  bool type_changed = not holds(CONSTANT);

  std::string name(get_name());

  parsed_data = cpp_f;

  if (type_changed)
    set_name(lhs_str);

  mathworld.object_updated(slot, false, name, std::string(get_name()));

  return *this;
}

template <parsing::Type type>
DynMathObject<type>& DynMathObject<type>::set(std::string_view name, double cst)
{
  std::string old_name(get_name());

  parsed_data = ConstObj{.val = cst};

  set_name_internal(name, name);

  mathworld.object_updated(slot, false, old_name, std::string(get_name()));

  return *this;
}

template <parsing::Type type>
DynMathObject<type>& DynMathObject<type>::operator = (double cst)
{
  bool type_changed = not holds(CONSTANT);

  parsed_data = ConstObj{.val = cst};

  if (type_changed)
  {
    set_name(lhs_str); // refresh the name to check for input variables
    mathworld.object_updated(slot, false, std::string(get_name()), std::string(get_name()));
  }

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

  auto ast = parsing::tokenize(definition)
               .and_then(parsing::make_ast{definition})
               .transform(parsing::flatten_separators);
  if (not ast)
    parsed_data = ast.error();

  // the root node of the tree must be the equal sign
  else if (not (ast->is_func() and ast->func_data().type == parsing::AST::Func::OP_ASSIGN))
    parsed_data = Error::not_math_object_definition();

  else
  {
    auto& funcop_data = ast->func_data();

    assert(funcop_data.subnodes.size() == 2);

    auto& rhs = funcop_data.subnodes[1];
    parsing::offset_tokens(rhs, -int(ast->name.begin));

    if (rhs.is_func() and rhs.func_data().type == parsing::AST::Func::SEPARATOR)
      parsed_data = SeqObj{.rhs_str = definition.substr(ast->name.begin),
                           .rhs = std::move(rhs.func_data().subnodes)};

    else if (rhs.is_number())
      parsed_data = ConstObj{.val = rhs.number_data().value,
                             .rhs_str = definition.substr(ast->name.begin)};

    else
      parsed_data = FuncObj{.rhs_str = definition.substr(ast->name.begin), .rhs = std::move(rhs)};

    set_name_internal(funcop_data.subnodes[0], definition.substr(0, ast->name.begin));
  }

  finalize_asts();

  mathworld.object_updated(slot,
                           std::holds_alternative<SeqObj>(parsed_data)
                             or std::holds_alternative<FuncObj>(parsed_data),
                           old_name,
                           std::string(get_name()));

  return *this;
}

template <parsing::Type type>
DynMathObject<type>::operator bool () const
{
  return bool(name_status()) and bool(object_status());
}

template <parsing::Type type>
tl::expected<Ok, zc::Error> DynMathObject<type>::name_status() const
{
  if (bool(exp_lhs))
    return Ok{};
  else return tl::unexpected(exp_lhs.error());
}

template <parsing::Type type>
tl::expected<Ok, zc::Error> DynMathObject<type>::object_status() const
{
  tl::expected<Ok, zc::Error> status = Ok{};
  std::visit(
    utils::overloaded{
      [&](const zc::Error& err) { status = tl::unexpected(err); },
      [&]<class T>(const T& f)
        requires utils::is_any_of<T, FuncObj, SeqObj>
      {
        if (not f.linked_rhs)
          status = tl::unexpected(f.linked_rhs.error());
      },
      [&]<class T>(const T&)
        requires (not utils::is_any_of<T, FuncObj, SeqObj>)
      {}
    },
    parsed_data
  );

  if (not bool(status) and not status.error().token.substr.empty() and (holds(FUNCTION) or holds(SEQUENCE)))
    status.error().token.begin += lhs_str.size();

  return status;
}

template <parsing::Type type>
tl::expected<Ok, zc::Error> DynMathObject<type>::status() const
{
  if (auto state = name_status(); not bool(state))
    return tl::unexpected(state.error());
  else if (auto state = object_status(); not bool(state))
    return tl::unexpected(state.error());
  else return Ok{};
}

template <parsing::Type type>
std::optional<zc::Error> DynMathObject<type>::error() const
{
  if (auto state = status(); not bool(state))
    return state.error();
  else return {};
}

template <parsing::Type type>
ObjectType DynMathObject<type>::object_type() const
{
  return std::visit(
    utils::overloaded{
      [&](const zc::Error&) { return BAD_EQUATION; },
      [&](const ConstObj&) { return CONSTANT; },
      [&]<size_t args_num>(const CppFunction<args_num>&){ return CPP_FUNCTION; },
      [&](const FuncObj&) { return FUNCTION; },
      [&](const SeqObj&) { return SEQUENCE; },
      [&](const DataObj&) { return DATA; }
    },
    parsed_data
  );
}

template <parsing::Type type>
tl::expected<typename DynMathObject<type>::LinkedRepr, zc::Error>
  DynMathObject<type>::get_linked_repr() const
{
  using RetT = tl::expected<typename DynMathObject<type>::LinkedRepr, zc::Error>;

  return std::visit(
    utils::overloaded{
      [&](const zc::Error& err) -> RetT { return tl::unexpected(err); },
      [&](const ConstObj& c) -> RetT { return &c.val; },
      [&]<size_t args_num>(CppFunction<args_num> f) -> RetT { return f; },
      [&](const FuncObj& f) -> RetT {
        if (f.linked_rhs) return &(*f.linked_rhs);
        else return tl::unexpected(f.linked_rhs.error());
      },
      [&](const SeqObj& u) -> RetT {
        if (u.linked_rhs) return &(*u.linked_rhs);
        else return tl::unexpected(u.linked_rhs.error());
      },
      [&](const DataObj& d) -> RetT { return &(d.linked_rhs); }
    },
    parsed_data
  );

}

template <parsing::Type type>
bool DynMathObject<type>::holds(ObjectType obj_type) const
{
  return object_type() == obj_type;
}

template <parsing::Type type>
DynMathObject<type>& DynMathObject<type>::set_data(std::string_view name, std::vector<std::string> data)
{
  std::string old_name(get_name());

  parsed_data = DataObj{.data = std::move(data)};

  DataObj& data_obj = std::get<DataObj>(parsed_data);
  data_obj.rhs.reserve(data.size());
  for (std::string_view expr: data_obj.data)
    data_obj.rhs.push_back(parsing::tokenize(expr)
                             .and_then(parsing::make_ast{expr})
                             .transform(parsing::flatten_separators));

  // should come before finalize_asts so the input variables are parsed
  set_name_internal(name, name);

  finalize_asts();

  mathworld.object_updated(slot,
                           true,
                           old_name,
                           std::string(get_name()));

  return *this;
}

template <parsing::Type type>
DynMathObject<type>& DynMathObject<type>::set_data_point(size_t index, std::string expr)
{
  std::string old_name(get_name());
  bool changed_type = not holds(DATA);
  if (changed_type)
  {
    parsed_data = DataObj{};
    set_name(lhs_str);
  }

  DataObj& data_obj = std::get<DataObj>(parsed_data);

  assert(data_obj.data.size() == data_obj.rhs.size());
  assert(data_obj.data.size() == data_obj.linked_rhs.repr.size());

  if (data_obj.data.size() <= index)
  {
    data_obj.data.resize(index+1);
    data_obj.rhs.resize(index+1, tl::unexpected(zc::Error::empty_expression()));
    data_obj.linked_rhs.repr.resize(index+1, tl::unexpected(zc::Error::empty_expression()));
  }

  data_obj.data[index] = std::move(expr);
  data_obj.rhs[index] = parsing::tokenize(data_obj.data[index])
                        .and_then(parsing::make_ast{data_obj.data[index]})
                        .transform(parsing::flatten_separators);

  data_obj.linked_rhs.repr[index] = data_obj.rhs[index].and_then(
    [&](auto&& ast)
    {
      return get_final_repr(ast, data_obj.data[index]);
    });

  if (changed_type)
    mathworld.object_updated(slot, true, old_name, std::string(get_name()));

  return *this;
}

template <parsing::Type type>
tl::expected<parsing::Parsing<type>, zc::Error>
  DynMathObject<type>::get_final_repr(const parsing::AST& ast, std::string_view equation)
{
  std::vector<std::string> var_names;
  if (bool(exp_lhs))
  {
    var_names.reserve(exp_lhs->input_vars.size());
    std::ranges::transform(exp_lhs->input_vars,
                           std::back_inserter(var_names),
                           &parsing::tokens::Text::substr);
  }

  auto final_ast = parsing::mark_input_vars{var_names}(ast);
  if constexpr (type == parsing::Type::FAST)
    return parsing::make_fast<type>{std::string(equation), mathworld}(final_ast);
  else
    return parsing::make_fast<type>{std::string(equation), mathworld}(final_ast).transform(
      parsing::make_RPN);
}

template <parsing::Type type>
template <bool linked>
DynMathObject<type>& DynMathObject<type>::finalize_asts()
{
  std::visit(
    utils::overloaded{
      [&](const zc::Error&) {},
      [&](const ConstObj&) {},
      [&]<size_t args_num>(const CppFunction<args_num>&) {},
      [&](FuncObj& f_obj)
      {
        f_obj.linked_rhs =
          parsing::LinkedFunc<type>{.repr = {},
                                    .args_num = exp_lhs
                                                ? exp_lhs->input_vars.size()
                                                : 0
          };
        if constexpr (linked)
        {
          auto exp_repr = get_final_repr(f_obj.rhs, lhs_str + f_obj.rhs_str);
          if (bool(exp_repr))
            f_obj.linked_rhs->repr = std::move(*exp_repr);
          else
            f_obj.linked_rhs = tl::unexpected(exp_repr.error());
        }
      },
      [&](SeqObj& seq_obj)
      {
        seq_obj.linked_rhs = parsing::LinkedSeq<type>{.repr = {},
                                                      .slot = slot};
        if constexpr (linked)
        {
          auto& values = seq_obj.linked_rhs->repr;
          values.reserve(seq_obj.rhs.size());
          for (const parsing::AST& ast : seq_obj.rhs)
          {
            auto exp_linked = get_final_repr(ast, lhs_str + seq_obj.rhs_str);
            if (bool(exp_linked))
              values.push_back(std::move(*exp_linked));
            else
            {
              seq_obj.linked_rhs = tl::unexpected(exp_linked.error());
              return;
            }
          }
        }
      },
      [&](DataObj& data_obj)
      {
        data_obj.linked_rhs.repr.clear();
        data_obj.linked_rhs.repr.reserve(data_obj.rhs.size());
        data_obj.linked_rhs.slot = slot;

        for (size_t i = 0; i != data_obj.rhs.size(); i++)
          data_obj.linked_rhs.repr.push_back(data_obj.rhs[i].and_then(
            [&](auto&& val)
            {
              return get_final_repr(val, data_obj.data[i]);
            }));
      }},
    parsed_data);

  return *this;
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
  if (not bool(exp_lhs))
    return {};

  else return std::visit(
    utils::overloaded{
      [&](const ConstObj& obj) -> std::optional<std::string>
      {
        if (not bool(obj.rhs_str))
          return {};

        return lhs_str + *obj.rhs_str;
      },
      [&]<class T>(const T& obj) -> std::optional<std::string>
        requires utils::is_any_of<T, FuncObj, SeqObj>
      {
        return lhs_str + obj.rhs_str;
      },
      []<class T>(const T&) -> std::optional<std::string>
        requires (not utils::is_any_of<T, FuncObj, SeqObj, ConstObj>)
      {
        return {};
      }
    },
    parsed_data
  );
}

template <parsing::Type type>
deps::Deps DynMathObject<type>::direct_dependencies() const
{
  return std::visit(
    utils::overloaded{
      [&](const zc::Error&)
      {
        return deps::Deps();
      },
      [&](const ConstObj&)
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
        auto deps = parsing::direct_dependencies(parsing::mark_input_vars{var_names}(f_obj.rhs));
        // offset the indexes with the current lhs
        for (auto& [name, dep]: deps)
          for(size_t& index: dep.indexes)
            index += lhs_str.size();
        return deps;
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
        // offset the indexes with the current lhs
        for (auto& [name, dep]: deps)
          for(size_t& index: dep.indexes)
            index += lhs_str.size();
        return deps;
      },
      [&](const DataObj& data_obj)
      {
        auto deps = deps::Deps();
        auto var_names = exp_lhs->input_vars | std::views::transform(&parsing::tokens::Text::substr);
        for (const auto& exp_ast: data_obj.rhs)
        {
          if (not bool(exp_ast))
            continue;

          auto extra_deps = parsing::direct_dependencies(parsing::mark_input_vars{var_names}(*exp_ast));
          deps.insert(extra_deps.begin(), extra_deps.end());
        }
        return deps;
      }
    }, parsed_data);
}

}
