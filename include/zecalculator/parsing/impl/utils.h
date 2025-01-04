#pragma once

/****************************************************************************
**  Copyright (c) 2023, Adel Kara Slimane <adel.ks@zegrapher.com>
**
**  This file is part of ZeCalculator.
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

#include <zecalculator/parsing/decl/parser.h>
#include <zecalculator/parsing/decl/utils.h>

namespace zc {
namespace parsing {

inline bool is_valid_name(std::string_view name)
{
  auto parsing = tokenize(name);
  return parsing and parsing->size() == 1 and parsing->front().type == tokens::VARIABLE;
}

/// @brief appends dependencies of 'ast' in 'deps'
struct direct_dependency_saver
{
  deps::Deps deps;

  direct_dependency_saver& operator () (const AST& ast)
  {
    std::visit(
      utils::overloaded{[&](const AST::Func& func)
                        {
                          // we don't register operators
                          if (func.type == AST::Func::FUNCTION)
                          {
                            deps::Dep& dep = deps[ast.name.substr];
                            dep.type = deps::Dep::FUNCTION;
                            dep.indexes.push_back(ast.name.begin);
                          }

                          std::ranges::for_each(func.subnodes, std::ref(*this));
                        },
                        [&](const AST::InputVariable&)
                        {
                        },
                        [&](const AST::Number&)
                        {
                        },
                        [&](AST::Variable)
                        {
                          deps::Dep& dep = deps[ast.name.substr];
                          dep.type = deps::Dep::VARIABLE;
                          dep.indexes.push_back(ast.name.begin);
                        }},
      ast.dyn_data);
    return *this;
  }
};

inline deps::Deps direct_dependencies(const AST& ast)
{
  return std::move(direct_dependency_saver{}(ast).deps);
}

/// @brief create LHS instance from a string representing the left hand side
inline tl::expected<LHS, zc::Error> parse_lhs(std::string_view lhs_expr, std::string_view full_expr)
{
  return parsing::tokenize(lhs_expr)
  .and_then(parsing::make_ast{full_expr})
  .transform(parsing::flatten_separators)
  .and_then([&](auto&& lhs){ return parse_lhs(lhs, full_expr); });
}

/// @brief create LHS instance from an already parsed string
inline tl::expected<LHS, zc::Error> parse_lhs(const AST& lhs, std::string_view full_expr)
{
  // can either be a variable or a function call
  if (lhs.is_var())
    return LHS{.name = lhs.name, .substr = lhs.name};

  else if (lhs.is_func())
  {
    const auto& func_data = lhs.func_data();
    auto res = LHS{.name = lhs.name, .substr = func_data.full_expr};

    if(func_data.type != parsing::AST::Func::FUNCTION)
      return tl::unexpected(Error::unexpected(lhs.name, std::string(full_expr)));

    // we don't handle functions with no input variables
    // they are used as variables instead
    assert(not func_data.subnodes.empty());

    res.input_vars.reserve(func_data.subnodes.size());
    for (const auto& arg: func_data.subnodes)
    {
      if (not arg.is_var()) [[ unlikely ]]
        return tl::unexpected(Error::unexpected(arg.name, std::string(full_expr)));
      else res.input_vars.push_back(arg.name);
    }

    return res;
  }
  else return tl::unexpected(Error::unexpected(lhs.name, std::string(full_expr)));
}

/// @brief changes the begin position of every token within the ast by 'offset'
inline void offset_tokens(AST& ast, int offset)
{
  ast.name.begin += offset;
  if (ast.is_func())
  {
    ast.func_data().full_expr.begin += offset;
    for (auto& node : ast.func_data().subnodes)
      offset_tokens(node, offset);
  }
}

} // namespace parsing
} // namespace zc
