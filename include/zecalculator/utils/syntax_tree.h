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

struct Error;

struct SyntaxTree
{
  // define types with the same value as the parsing ones
  enum Type {
    FUNCTION = Token::FUNCTION,
    NUMBER   = Token::NUMBER,
    VARIABLE = Token::VARIABLE
  };

  Type type;
  std::string str;
  std::optional<double> value = {};
  std::vector<SyntaxTree> subnodes = {};

  bool operator == (const SyntaxTree& other) const = default;
};

/// @brief creates a SyntaxNode from a parsing
tl::expected<SyntaxTree, Error> make_tree(const std::span<Token> tokens);

}
