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

#pragma once

#include <zecalculator/parsing/decl/parser.h>
#include <zecalculator/parsing/data_structures/decl/ast.h>

namespace zc {

namespace parsing {

/// @brief tells if the string_view contains a valid math object name
inline bool is_valid_name(std::string_view name);

/// @brief gives the Function and Variable names that intervene in this AST
inline deps::Deps direct_dependencies(const AST& ast);

/// @brief represents the left hand side of a mathematical definition through an equation
/// @example "var" in "var = cos(x)" -> {.name = "var", .input_vars = {}}
/// @example "f(x,y)" in "f(x,y) = 1+ cos(x)*cos(y)" -> {.name = "f", .input_vars = {"x", "y"}}
struct LHS
{
  tokens::Text name;
  std::vector<tokens::Text> input_vars = {};

  /// @brief the text that defines the LHS, without an leading or trailing whitespaces
  tokens::Text substr;

  bool operator == (const LHS&) const = default;
};

/// @brief changes the begin position of every token within the ast by 'offset'
void offset_tokens(AST& ast, int offset);

/// @brief create LHS instance from a string representing the left hand side
/// @arg lhs: substring where lhs is defined
/// @arg full_expr: full expression where 'lhs' appears, only used for errors
tl::expected<LHS, zc::Error> parse_lhs(std::string_view lhs, std::string_view full_expr);

/// @brief create LHS instance from an already parsed string
/// @arg lhs: the parsed lhs to use
/// @arg full_expr: full expression where 'lhs' appears, only used for errors
tl::expected<LHS, zc::Error> parse_lhs(const AST& lhs, std::string_view full_expr);

} // namespace parsing
} // namespace zc
