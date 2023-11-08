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
#include <cassert>
#include <cmath>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include <zecalculator/external/expected.h>
#include <zecalculator/utils/substr_info.h>

namespace zc {
namespace parsing {

namespace tokens {

struct Text
{
  Text() = default;

  Text(std::string_view name) : name(name)
  {}

  Text(std::string name) : name(std::move(name))
  {}

  Text(std::string_view substr, std::string_view original_expr)
    : name(std::string(substr)), substr_info(SubstrInfo::from_views(substr, original_expr))
  {}

  Text(std::string_view name, size_t begin)
    : name(std::string(name)), substr_info(SubstrInfo{begin, name.size()})
  {}

  Text(std::string_view name, size_t begin, size_t size)
    : name(std::string(name)), substr_info(SubstrInfo{begin, size})
  {}

  Text(std::string_view name, std::optional<SubstrInfo> substr_info)
    : name(std::string(name)), substr_info(substr_info)
  {}

  /// @brief name of the token, can different from what appears in the expressions
  /// @example '+' is replaced with 'internal::plus' (a valid function name)
  std::string name = {};

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
      name = t1.name + t2.name;
  }

  return Text(name, opt_substr_info);
}

struct Unkown: Text
{
  explicit Unkown(const Text& txt) : Text(txt) {}
  using Text::Text;
};

struct Number: Text
{
  Number(double value, const tokens::Text& text_token): Text(text_token), value(value) {}
  double value = std::nan("");
};

struct Variable: Text
{
  explicit Variable(const Text& txtTok): Text(txtTok) {}
  using Text::Text;
};

struct Function: Text
{
  explicit Function(const Text& txtTok): Text(txtTok) {}
  using Text::Text;
};

// operators ordered in increasing order of priority
inline constexpr std::array operators = {'=', '+', '-', '*', '/', '^'};

using OperatorSequence = std::integer_sequence<char, '=', '+', '-', '*', '/', '^'>;

inline constexpr bool is_operator(const char ch)
{
  return std::ranges::count(operators, ch);
}

template <char op, size_t args_num>
  requires (is_operator(op) and args_num >= 1)
struct Operator: Function
{
  Operator(size_t begin): Function(std::string(1, op), begin) {}

  Operator(std::string_view op_v, std::string_view original_expr)
    : Function(std::string(op_v), SubstrInfo::from_views(op_v, original_expr))
  {}
};

template <char op>
using BinaryOperator = Operator<op, 2>;

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
using TokenType = std::variant<tokens::Unkown,
                               tokens::Number,
                               tokens::Variable,
                               tokens::Function,
                               tokens::Operator<'=', 2>,
                               tokens::Operator<'+', 2>,
                               tokens::Operator<'-', 2>,
                               tokens::Operator<'*', 2>,
                               tokens::Operator<'/', 2>,
                               tokens::Operator<'^', 2>,
                               tokens::OpeningParenthesis,
                               tokens::ClosingParenthesis,
                               tokens::FunctionCallStart,
                               tokens::FunctionCallEnd,
                               tokens::FunctionArgumentSeparator,
                               tokens::EndOfExpression>;

struct Token: TokenType
{
  using TokenType::TokenType;
};

inline tokens::Text text_token(const Token& token)
{
  return std::visit([](const auto& tk) -> tokens::Text { return tk; }, token);
}

inline std::optional<SubstrInfo> substr_info(const Token& token)
{
  return text_token(token).substr_info;
}

}
}
