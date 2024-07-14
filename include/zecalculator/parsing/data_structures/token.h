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

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <variant>

#include <zecalculator/external/expected.h>
#include <zecalculator/utils/tuple.h>
#include <zecalculator/utils/utils.h>

namespace zc {
namespace parsing {

namespace tokens {

struct Text
{

  static Text from_views(std::string_view substr, std::string_view full_str)
  {
    return Text{.substr = std::string(substr), .begin = utils::begin_index(substr, full_str)};
  };

  /// @brief name of the token, can different from what appears in the expressions
  /// @example '+' is replaced with 'internal::plus' (a valid function name)
  std::string substr = {};

  ///@brief begin position of 'substr' in the original string
  size_t begin = 0;

  bool operator == (const Text& other) const = default;
};

enum Type: size_t
{
  UNKNOWN,
  NUMBER,
  VARIABLE,
  FUNCTION,
  OP_ASSIGN,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_POWER,
  OP_UNARY_PLUS,
  OP_UNARY_MINUS,
  OPENING_PARENTHESIS,
  CLOSING_PARENTHESIS,
  FUNCTION_CALL_START,
  FUNCTION_CALL_END,
  SEPARATOR,
  END_OF_EXPRESSION,
};

struct Operator
{
  enum Desc {BINARY_INFIX, UNARY_PREFIX};

  char token;
  uint8_t priority;
  Type type;
  Desc desc;

};

inline constexpr std::array operators = {
  Operator{'=', 0, OP_ASSIGN, Operator::BINARY_INFIX},
  Operator{',', 1, SEPARATOR, Operator::BINARY_INFIX},
  Operator{';', 1, SEPARATOR, Operator::BINARY_INFIX},
  Operator{'+', 2, OP_ADD, Operator::BINARY_INFIX},
  Operator{'-', 2, OP_SUBTRACT, Operator::BINARY_INFIX},
  Operator{'*', 3, OP_MULTIPLY, Operator::BINARY_INFIX},
  Operator{'/', 3, OP_DIVIDE, Operator::BINARY_INFIX},
  Operator{'-', 4, OP_UNARY_MINUS, Operator::UNARY_PREFIX},
  Operator{'+', 4, OP_UNARY_PLUS, Operator::UNARY_PREFIX},
  Operator{'^', 5, OP_POWER, Operator::BINARY_INFIX},
};

inline constexpr uint8_t max_priority = std::ranges::max(operators | std::views::transform(&Operator::priority));

consteval size_t operator_number(size_t priority)
{
  size_t num = 0;
  for (const auto& op: operators)
    if (op.priority == priority)
      num++;
  return num;
}

template <uint8_t priority>
consteval auto get_operators()
{
  std::array<Operator, operator_number(priority)> ops;
  size_t i = 0;
  for (const auto& op: operators)
    if (op.priority == priority)
    {
      ops[i] = op;
      i++;
    }

  // just a "constexpr" check
  if (i != ops.size())
    throw 0;

  return ops;
}

inline constexpr std::optional<Operator> as_binary_infix_operator(const char ch)
{
  for(const auto& op: operators)
    if (op.desc == Operator::BINARY_INFIX and op.token == ch)
      return op;

  return {};
}

inline constexpr std::optional<Operator> as_unary_prefix_operator(const char ch)
{
  for(const auto& op: operators)
    if (op.desc == Operator::UNARY_PREFIX and op.token == ch)
      return op;

  return {};
}

}

struct Token: tokens::Text
{
  Token(tokens::Type type, tokens::Text text)
    : tokens::Text{std::move(text)}, type(type)
  {}

  Token(double value, tokens::Text text)
    : tokens::Text{std::move(text)}, type(tokens::NUMBER), value(value)
  {}

  static Token OpeningParenthesis(std::string_view name, size_t start) {
    return Token(tokens::OPENING_PARENTHESIS, Text{std::string(name), start});
  }

  static Token Function(std::string_view name, size_t start) {
    return Token(tokens::FUNCTION, Text{std::string(name), start});
  }

  static Token FunctionCallStart(std::string_view name, size_t start) {
    return Token(tokens::FUNCTION_CALL_START, Text{std::string(name), start});
  }

  static Token Variable(std::string_view name, size_t start) {
    return Token(tokens::VARIABLE, Text{std::string(name), start});
  }
  static Token FunctionCallEnd(std::string_view name, size_t start) {
    return Token(tokens::FUNCTION_CALL_END, Text{std::string(name), start});
  }

  static Token Number(double value, std::string_view name, size_t start) {
    return Token(value, Text{std::string(name), start});
  }

  static Token Number(double value, tokens::Text token) {
    return Token(value, std::move(token));
  }

  static Token Separator(std::string_view name, size_t start) {
    return Token(tokens::SEPARATOR, Text{std::string(name), start});
  }

  static Token ClosingParenthesis(std::string_view name, size_t start) {
    return Token(tokens::CLOSING_PARENTHESIS, Text{std::string(name), start});
  }

  static Token Assign(std::string_view name, size_t start) {
    return Token(tokens::OP_ASSIGN, Text{std::string(name), start});
  }

  static Token Add(std::string_view name, size_t start) {
    return Token(tokens::OP_ADD, Text{std::string(name), start});
  }

  static Token Subtract(std::string_view name, size_t start) {
    return Token(tokens::OP_SUBTRACT, Text{std::string(name), start});
  }

  static Token Multiply(std::string_view name, size_t start) {
    return Token(tokens::OP_MULTIPLY, Text{std::string(name), start});
  }

  static Token Divide(std::string_view name, size_t start) {
    return Token(tokens::OP_DIVIDE, Text{std::string(name), start});
  }

  static Token UnaryMinus(std::string_view name, size_t start) {
    return Token(tokens::OP_UNARY_MINUS, Text{std::string(name), start});
  }

  static Token UnaryPlus(std::string_view name, size_t start) {
    return Token(tokens::OP_UNARY_MINUS, Text{std::string(name), start});
  }

  static Token Power(std::string_view name, size_t start) {
    return Token(tokens::OP_POWER, Text{std::string(name), start});
  }

  tokens::Type type = tokens::UNKNOWN;

  double value = std::nan("");
};

template <class... U>
inline tokens::Text text_token(const std::variant<U...>& token)
{
  return std::visit([](const auto& tk) -> tokens::Text { return tk; }, token);
}

}
}
