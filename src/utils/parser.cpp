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

#include <cassert>
#include <zecalculator/utils/parser.h>

#include <charconv>
#include <optional>
#include <string_view>
#include <array>
#include <ranges>
#include <algorithm>
#include <sstream>
#include <stack>

namespace zc {
namespace parsing {

std::optional<std::pair<double, size_t>> to_double(std::string_view view)
{
  std::optional<std::pair<double, size_t>> result = std::make_pair(0.0, 0);
  auto [ptr, ec] = std::from_chars(view.data(), view.data() + view.size(), result->first);

  if (ec == std::errc())
    result->second = size_t(ptr - view.data());
  else result.reset();

  return result;
}

Token::Operator::Operator(const char op_char)
{
  switch(op_char)
  {
  case '+':
    op = PLUS;
    break;
  case '-':
    op = MINUS;
    break;
  case '*':
    op = MULTIPLY;
    break;
  case '/':
    op = DIVIDE;
    break;
  case '^':
    op = POWER;
    break;
  [[unlikely]] default:
    throw std::invalid_argument("not an operator");
  }
}


char Token::Operator::name() const
{
  switch(op)
  {
  case PLUS:
    return '+';
  case MINUS:
    return '-';
  case MULTIPLY:
    return '*';
  case DIVIDE:
    return '/';
  case POWER:
    return '^';
  [[unlikely]] default:
    throw std::invalid_argument("not an operator");
  }
}

Token::Token(Token::Type type,
             std::optional<std::variant<Token::Operator, double, std::string>> type_value)
  : type(type)
{
  switch(type)
  {
  case Type::NUMBER:
    value = std::get<double>(type_value.value());
    break;
  case Type::VARIABLE: [[fallthrough]];
  case Type::FUNCTION:
    name = std::get<std::string>(type_value.value());
    break;
  case Type::OPERATOR:
    op = std::get<Operator>(type_value.value());
    break;
  default: ;
  }
}

std::ostream& operator << (std::stringstream& os, const Token& token)
{
  switch(token.type)
  {
  case Token::Type::NUMBER:
    os << std::to_string(token.value.value());
    break;
  case Token::Type::VARIABLE:
    os << "var=" << token.name.value();
    break;
  case Token::Type::FUNCTION:
    os << "func=" << token.name.value();
    break;
  case Token::Type::OPERATOR:
    os << token.op.value().name();
    break;
  default:
    os << token.type_name();
  }

  return os;
}

std::pair<std::vector<Token>, std::optional<Error>> parse(std::string expression)
{
  std::pair<std::vector<Token>, std::optional<Error>> result;
  auto& parsing = result.first;

  auto error_out = [&](Error::Type error_type, std::string_view where)
  {
    result.second.emplace(Error
    {
      .type = error_type,
      .expression = std::move(expression),
      .where = where
    });

    return result;
  };

  auto is_operator = [](const char ch) {
    static constexpr std::array operators = {'+', '-', '*', '/', '^'};
    return std::ranges::any_of(operators, [&ch](const char op){ return op == ch; });
  };

  auto is_seperator = [](const char ch) {
    static constexpr std::array separators = {'+', '-', '*', '/', '^', ' ', '(', ')'};
    return std::ranges::any_of(separators, [&ch](const char op){ return op == ch; });
  };

  auto is_digit = [](const char ch)
  {
    return std::isdigit(static_cast<unsigned char>(ch));
  };

  bool openingParenthesis = true, numberSign = true, value = true, canEnd = false,
       ope = false, closingParenthesis = false;

  enum : bool { FUNCTION_CALL_PTH, NORMAL_PTH};
  std::stack<bool> last_opened_pth;

  auto it = expression.cbegin();
  while (it != expression.cend())
  {
    std::optional<char> next_char = it+1 != expression.cend() ? *(it+1) : std::optional<char>();
    if (is_digit(*it) or (numberSign and *it == '-' and next_char and is_digit(next_char.value())))
    {
      auto double_val = to_double(std::string_view(it, expression.cend()));

      if (double_val)
      {
        const auto& [double_opt_val, processed_char_num] = *double_val;
        // parsing successful
        parsing.emplace_back(Token::Type::NUMBER, double_opt_val);
        it += processed_char_num;

        openingParenthesis = value = numberSign = false;
        ope = canEnd = closingParenthesis = true;
      }
      else return error_out(Error::WRONG_NUMBER_FORMAT, std::string_view(it, it+1));
    }
    else if (is_operator(*it))
    {
      if (ope)
      {
        parsing.emplace_back(Token::Type::OPERATOR, *it);

        openingParenthesis = value = true;
        ope = numberSign = closingParenthesis = canEnd = false;
        it++;
      }
      else return error_out(Error::UNEXPECTED_OPERATOR, std::string_view(it, it+1));
    }
    else if (*it == '(')
    {
      if (openingParenthesis)
      {
        if (not parsing.empty() and parsing.back().type == Token::Type::FUNCTION)
        {
          parsing.emplace_back(Token::Type::FUNCTION_CALL_START);
          last_opened_pth.push(FUNCTION_CALL_PTH);
        }
        else
        {
          parsing.emplace_back(Token::Type::OPENING_PARENTHESIS);
          last_opened_pth.push(NORMAL_PTH);
        }

        numberSign = value = openingParenthesis = true;
        ope = closingParenthesis = canEnd = false;
        it++;
      }
      else return error_out(Error::UNEXPECTED_OPENING_PARENTHESIS, std::string_view(it, it+1));
    }
    else if (*it == ')')
    {
      if (closingParenthesis and not last_opened_pth.empty())
      {
        if (last_opened_pth.top() == FUNCTION_CALL_PTH)
        {
          parsing.emplace_back(Token::Type::FUNCTION_CALL_END);
        }
        else parsing.emplace_back(Token::Type::CLOSING_PARENTHESIS);

        last_opened_pth.pop();

        ope = canEnd = closingParenthesis = true;
        value = numberSign = openingParenthesis = false;
        it++;
      }
      else return error_out(Error::UNEXPECTED_CLOSING_PARENTHESIS, std::string_view(it, it+1));
    }
    else if (*it == ' ')
      // spaces are skipped
      it++;
    else
    {
      if (value)
      {
        // the only possibilities left are variables and functions
        // to differentiate between them, functions always are followed by an opening parenthesis
        // e.g. "cos("

        auto token_begin = it;
        while (it != expression.cend() and
               not is_seperator(*it)) { it++; }

        std::string_view token(token_begin, it);

        if (it == expression.cend() or *it != '(')
        {
          // can only be a variable when we reach the end of the expression
          parsing.emplace_back(Token::Type::VARIABLE, std::string(token));

          openingParenthesis = numberSign = value = false;
          canEnd = ope = closingParenthesis = true;
        }
        else
        {
          parsing.emplace_back(Token::Type::FUNCTION, std::string(token));

          canEnd = closingParenthesis = ope = numberSign = value = false;
          openingParenthesis = true;
        }
      }
      else return error_out(Error::UNEXPECTED_VARIABLE_OR_FUNCTION, std::string_view(it, it+1));
    }
  }

  if (not last_opened_pth.empty())
  {
    if (last_opened_pth.top() == FUNCTION_CALL_PTH)
      return error_out(Error::MISSING_CLOSING_FUNCTION_CALL, std::string_view(it-1, it));
    else return error_out(Error::MISSING_CLOSING_PARENTHESES, std::string_view(it-1, it));
  }

  if (not canEnd)
    return error_out(Error::UNEXPTECTED_END_OF_EXPRESSION, std::string_view(it-1, it));

  return result;
}

}
}
