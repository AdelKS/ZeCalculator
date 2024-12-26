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

} // namespace parsing
} // namespace zc
