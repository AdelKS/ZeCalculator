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

#include <optional>
#include <span>
#include <vector>

#include <zecalculator/utils/parser.h>

namespace zc {

struct ParsingError;
struct FunctionNode;
struct VariableNode;
struct NumberNode;

using SyntaxTree = std::variant<std::monostate, FunctionNode, VariableNode, NumberNode>;

struct VariableNode: tokens::Text
{
  VariableNode(const tokens::Text& text): tokens::Text(text) {}

  using tokens::Text::Text;

  bool operator == (const VariableNode& other) const = default;
};

struct NumberNode: tokens::Number
{
  NumberNode(const tokens::Number& number_token) : tokens::Number(number_token) {}

  using tokens::Number::Number;
};

struct FunctionNode: tokens::Text
{
  FunctionNode(const tokens::Text& text, std::vector<SyntaxTree> subnodes)
    : tokens::Text(text), subnodes(std::move(subnodes))
  {}

  std::vector<SyntaxTree> subnodes;

  bool operator == (const FunctionNode& other) const = default;
};

/// @brief makes a syntax tree from from a sequence of tokens
/// @note  the tokens contain string_views to the original expression
///        i.e. be cautious when using it
tl::expected<SyntaxTree, ParsingError> make_tree(const std::span<Token> tokens);

}
