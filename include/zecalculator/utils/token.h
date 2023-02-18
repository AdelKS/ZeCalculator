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

#include <vector>
#include <optional>
#include <variant>
#include <cstdint>
#include <optional>
#include <string_view>
#include <cassert>
#include <algorithm>
#include <ranges>

#include <zecalculator/external/expected.h>

namespace zc {

/// @brief represents a token in a parsed expression
/// @example an operatr '+', a function name 'cos', a variable 'x', a number '-3.14E+2'
struct Token
{
  using Operator = char;

  // operators ordered in increasing order of priority
  static constexpr std::array operators = {'+', '-', '*', '/', '^'};

  constexpr static bool is_operator(const char ch)
  {
    return std::ranges::any_of(operators, [&ch](const char op){ return op == ch; });
  }

  enum Type : uint8_t
  {
    UNKNOWN = 0,
    NUMBER,
    VARIABLE,
    FUNCTION,
    OPERATOR,
    OPENING_PARENTHESIS,
    CLOSING_PARENTHESIS,
    FUNCTION_CALL_START, // opening parenthesis for a function call
    FUNCTION_CALL_END, // closing parenthesis for a function call
    FUNCTION_ARGUMENT_SEPARATOR, // e.g. the ',' in 'pow(x, y)'
    END_OF_EXPRESSION,
  };

  bool operator == (const Token& other) const = default;

  Type type = UNKNOWN;
  std::string_view str_v = {}; // string view on the token's text within the original expression
  std::variant<std::monostate, Operator, double> type_value = {};
};

}
