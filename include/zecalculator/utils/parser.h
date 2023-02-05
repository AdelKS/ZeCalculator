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

#include <zecalculator/utils/error.h>

/* TODO: update approach as the following:
    - Parse: aka cut each atom in a formula
    - Evaluate type of atom: separator, number
    - Treat number in a special as to make 1.2E+33 as one atom
    - Check for validity
    - Enable setting custom names for functions and variables
    - Performance improvement: flatten trees
*/

#include <optional>
#include <string_view>

namespace zc {

/// @brief interprets "view" as a floating number
/// @returns if successful, the interpreted double and the number of characters interpreted, otherwise empty
std::optional<std::pair<double, size_t>> to_double(std::string_view view);

namespace parsing
{

/// @brief represents a token in a parsed expression
/// @example an operatr '+', a function name 'cos', a variable 'x', a number '-3.14E+2'
struct Token
{
  struct Operator
  {
    enum Type {PLUS, MINUS, MULTIPLY, DIVIDE, POWER};
    Type op;

    bool operator == (const Operator &other) const = default;

    Operator(Type op_type): op(op_type) {}

    /// @brief builds from char, throws if it's not an operator
    Operator(const char op_char);

    /// @brief returns the char that corresponds to the operator
    char name() const;
  };

  enum Type : uint8_t
  {
    NUMBER = 0,
    VARIABLE,
    FUNCTION,
    OPERATOR,
    OPENING_PARENTHESIS,
    CLOSING_PARENTHESIS,
    FUNCTION_CALL_START, // opening parenthesis for a function call
    FUNCTION_CALL_END, // closing parenthesis for a function call
    FUNCTION_ARGUMENT_SEPARATOR // e.g. the ',' in 'pow(x, y)'
  };

  static constexpr std::array type_to_str_map =
  {
    "NUMBER",
    "VARIABLE",
    "FUNCTION",
    "OPERATOR",
    "OPENING_PARENTHESIS",
    "CLOSING_PARENTHESIS",
    "FUNCTION_CALL_START",
    "FUNCTION_CALL_END",
    "FUNCTION_ARGUMENT_SEPARATOR"
  };

  /// @brief returns he name of the type 'type'
  std::string_view type_name() const
  {
    assert(type < type_to_str_map.size());
    return type_to_str_map[type];
  }

  Token(Type type, std::optional<std::variant<Operator, double, std::string>> type_value = {});



  bool operator == (const Token& other) const = default;

  friend std::ostream& operator << (std::stringstream& os, const Token& token);

  Type type;
  std::optional<Operator> op = std::optional<Operator>();
  std::optional<double> value = std::optional<double>();
  std::optional<std::string> name = std::optional<std::string>();
};

std::pair<std::vector<Token>, std::optional<Error>> parse(std::string expression);

};

}
