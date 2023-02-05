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

#include <string_view>
#include <string>
#include <array>
#include <cassert>

namespace zc
{

struct Error
{
  // type of error
  enum Type : uint8_t
  {
    UNDEFINED = 0,
    WRONG_NUMBER_FORMAT,
    UNEXPECTED_OPERATOR,
    UNEXPECTED_OPENING_PARENTHESIS,
    UNEXPECTED_CLOSING_PARENTHESIS,
    UNEXPECTED_VARIABLE_OR_FUNCTION,
    UNEXPTECTED_END_OF_EXPRESSION,
    MISSING_CLOSING_PARENTHESES,
    MISSING_CLOSING_FUNCTION_CALL,
  };

  static constexpr std::array error_to_str_map =
  {
    "UNDEFINED",
    "WRONG_NUMBER_FORMAT",
    "UNEXPECTED_OPERATOR",
    "UNEXPECTED_OPENING_PARENTHESIS",
    "UNEXPECTED_CLOSING_PARENTHESIS",
    "UNEXPECTED_VARIABLE_OR_FUNCTION",
    "UNEXPTECTED_END_OF_EXPRESSION",
    "MISSING_CLOSING_PARENTHESES",
    "MISSING_CLOSING_FUNCTION_CALL",
  };

  /// @brief returns he name of the type 'type'
  std::string_view error_name() const
  {
    assert(type < error_to_str_map.size());
    return error_to_str_map[type];
  }

  Type type = UNDEFINED;

  // the expression
  std::string expression;

  // where in the expression above the error happenned
  std::string_view where;
};

}
