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

#include <cmath>
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

namespace tokens {

struct Text
{
  Text() = default;
  Text(std::string_view str_v) : str_v(str_v) {}
  std::string_view str_v = {}; // string view on the 's text within the original expression

  bool operator == (const Text& other) const = default;
};

struct Unkown: Text
{
  using Text::Text;
};

struct Number: Text
{
  Number(double value, std::string_view str_v): Text(str_v), value(value) {};
  double value = std::nan("");
};

struct Variable: Text
{
  using Text::Text;
};

struct Function: Text
{
  using Text::Text;
};

struct Operator: Text
{
  using pair_type = std::pair<char, std::string_view>;
  // operators ordered in increasing order of priority
  static constexpr std::array<pair_type, 5> operators = {{{'+', "internal::plus"},
                                                          {'-', "internal::minus"},
                                                          {'*', "internal::multiply"},
                                                          {'/', "internal::divide"},
                                                          {'^', "internal::power"}}};

  template <char op> requires (std::ranges::count(operators, op, &pair_type::first) == 1)
  consteval static std::string_view name_of()
  {
    return std::ranges::find(operators, op, &pair_type::first)->second;
  }

  constexpr static bool is_operator(const char ch)
  {
    return std::ranges::any_of(
      operators, [&ch](const char op) { return op == ch; }, &pair_type::first);
  }

  Operator(std::string_view str_v): Text(str_v)
  {
    assert(str_v.size() == 1 and is_operator(str_v.front()));
  }
};

struct OpeningParenthesis: Text
{
  using Text::Text;
};

struct ClosingParenthesis: Text
{
  using Text::Text;
};

struct FunctionCallStart: Text
{
  using Text::Text;
};

struct FunctionCallEnd: Text
{
  using Text::Text;
};

struct FunctionArgumentSeparator: Text
{
  using Text::Text;
};

struct EndOfExpression: Text // will be used only to signal errors
{
  using Text::Text;
};

}

/// @brief represents a  in a parsed expression
/// @example an operatr '+', a function name 'cos', a variable 'x', a number '-3.14E+2'
using TokenType =
    std::variant<tokens::Unkown, tokens::Number, tokens::Variable,
                 tokens::Function, tokens::Operator, tokens::OpeningParenthesis,
                 tokens::ClosingParenthesis, tokens::FunctionCallStart,
                 tokens::FunctionCallEnd, tokens::FunctionArgumentSeparator,
                 tokens::EndOfExpression>;

struct Token: TokenType
{
  using TokenType::TokenType;
};

}
