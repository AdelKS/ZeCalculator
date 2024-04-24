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
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>

#include <zecalculator/external/expected.h>
#include <zecalculator/utils/substr_info.h>
#include <zecalculator/utils/tuple.h>

namespace zc {
namespace parsing {

namespace tokens {

struct Text
{
  Text() = default;

  Text(std::string_view name) : substr(name)
  {}

  Text(std::string name) : substr(std::move(name))
  {}

  Text(std::string_view substr, std::string_view original_expr)
    : substr(std::string(substr)), substr_info(SubstrInfo::from_views(substr, original_expr))
  {}

  Text(std::string_view name, size_t begin)
    : substr(std::string(name)), substr_info(SubstrInfo{begin, name.size()})
  {}

  Text(std::string_view name, size_t begin, size_t size)
    : substr(std::string(name)), substr_info(SubstrInfo{begin, size})
  {}

  Text(std::string_view name, std::optional<SubstrInfo> substr_info)
    : substr(std::string(name)), substr_info(substr_info)
  {}

  /// @brief name of the token, can different from what appears in the expressions
  /// @example '+' is replaced with 'internal::plus' (a valid function name)
  std::string substr = {};

  /// @brief information about the location of the token within the original expression
  /// @example token '+' in '2+2*2' will have: begin=1, size=1
  /// @note the SubstrInfo cannot be known sometimes
  std::optional<SubstrInfo> substr_info = {};

  bool operator == (const Text& other) const = default;
};

Text operator + (const Text& t1, const Text& t2)
{
  std::string name;
  std::optional<SubstrInfo> opt_substr_info;
  if (t1.substr_info and t2.substr_info)
  {
    opt_substr_info = *t1.substr_info + *t2.substr_info;

    if (t1.substr_info->begin + t1.substr_info->size == t2.substr_info->begin)
      name = t1.substr + t2.substr;
  }

  return Text(name, opt_substr_info);
}

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
  OPENING_PARENTHESIS,
  CLOSING_PARENTHESIS,
  FUNCTION_CALL_START,
  FUNCTION_CALL_END,
  SEPARATOR,
  END_OF_EXPRESSION,
};

struct Operator
{
  char token;
  uint8_t priority;
  Type type;
};

inline constexpr std::array operators = {Operator{'=', 0, OP_ASSIGN},
                                         Operator{'+', 1, OP_ADD},
                                         Operator{'-', 1, OP_SUBTRACT},
                                         Operator{'*', 2, OP_MULTIPLY},
                                         Operator{'/', 2, OP_DIVIDE},
                                         Operator{'^', 3, OP_POWER},};

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
  std::array<Type, operator_number(priority)> ops;
  size_t i = 0;
  for (const auto& op: operators)
    if (op.priority == priority)
    {
      ops[i] = op.type;
      i++;
    }

  // just a "constexpr" check
  if (i != ops.size())
    throw 0;

  return ops;
}

inline constexpr std::optional<Operator> get_operator_description(const char ch)
{
  for(const auto& op: operators)
    if (op.token == ch)
      return op;

  return {};
}

}

struct Token: tokens::Text
{
  Token(tokens::Type type, tokens::Text text)
    : tokens::Text(std::move(text)), type(type)
  {}

  Token(double value, tokens::Text text)
    : tokens::Text(std::move(text)), type(tokens::NUMBER), value(value)
  {}

  static Token OpeningParenthesis(std::string_view name, size_t start) {
    return Token(tokens::OPENING_PARENTHESIS, Text(name, start));
  }

  static Token Function(std::string_view name, size_t start) {
    return Token(tokens::FUNCTION, Text(name, start));
  }

  static Token FunctionCallStart(std::string_view name, size_t start) {
    return Token(tokens::FUNCTION_CALL_START, Text(name, start));
  }

  static Token Variable(std::string_view name, size_t start) {
    return Token(tokens::VARIABLE, Text(name, start));
  }
  static Token FunctionCallEnd(std::string_view name, size_t start) {
    return Token(tokens::FUNCTION_CALL_END, Text(name, start));
  }

  static Token Number(double value, std::string_view name, size_t start) {
    return Token(value, Text(name, start));
  }

  static Token Number(double value, tokens::Text token) {
    return Token(value, std::move(token));
  }

  static Token Separator(std::string_view name, size_t start) {
    return Token(tokens::SEPARATOR, Text(name, start));
  }

  static Token ClosingParenthesis(std::string_view name, size_t start) {
    return Token(tokens::CLOSING_PARENTHESIS, Text(name, start));
  }

  static Token Assign(std::string_view name, size_t start) {
    return Token(tokens::OP_ASSIGN, Text(name, start));
  }

  static Token Add(std::string_view name, size_t start) {
    return Token(tokens::OP_ADD, Text(name, start));
  }

  static Token Subtract(std::string_view name, size_t start) {
    return Token(tokens::OP_SUBTRACT, Text(name, start));
  }

  static Token Multiply(std::string_view name, size_t start) {
    return Token(tokens::OP_MULTIPLY, Text(name, start));
  }

  static Token Divide(std::string_view name, size_t start) {
    return Token(tokens::OP_DIVIDE, Text(name, start));
  }

  static Token Power(std::string_view name, size_t start) {
    return Token(tokens::OP_POWER, Text(name, start));
  }

  static Token EndOfExpression(size_t pos)
  {
    return Token(tokens::END_OF_EXPRESSION, Text("", pos));
  }

  tokens::Type type = tokens::UNKNOWN;

  double value = std::nan("");
};

template <class... U>
inline tokens::Text text_token(const std::variant<U...>& token)
{
  return std::visit([](const auto& tk) -> tokens::Text { return tk; }, token);
}

inline std::optional<SubstrInfo> substr_info(const Token& token)
{
  return token.substr_info;
}

}
}
