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
#include <cmath>
#include <zecalculator/utils/parsing_error.h>
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

std::optional<std::pair<double, size_t>> to_double(std::string_view view)
{
  std::optional<std::pair<double, size_t>> result = std::make_pair(0.0, 0);
  auto [ptr, ec] = std::from_chars(view.data(), view.data() + view.size(), result->first);

  if (ec == std::errc())
    result->second = size_t(ptr - view.data());
  else result.reset();

  return result;
}

tl::expected<std::vector<Token>, ParsingError> parse(std::string_view expression)
{
  std::vector<Token> parsing;

  auto is_seperator = [](const char ch) {
    static constexpr std::array separators = {'+', '-', '*', '/', '^', ' ', '(', ')'};
    return std::ranges::any_of(separators, [&ch](const char op){ return op == ch; });
  };

  auto is_digit = [](const char ch)
  {
    return std::isdigit(static_cast<unsigned char>(ch));
  };

  auto is_argument_separator = [](const char ch)
  {
    return ch == ',' or ch == ';';
  };

  bool openingParenthesis = true, numberSign = true, value = true, canEnd = false,
       ope = false, closingParenthesis = false;

  enum : bool { FUNCTION_CALL_PTH, NORMAL_PTH};
  std::stack<bool> last_opened_pth;

  auto it = expression.cbegin();
  while (it != expression.cend())
  {
    std::optional<char> next_char = it+1 != expression.cend() ? *(it+1) : std::optional<char>();

    // view on the single character pointed to by 'it'
    std::string_view char_v = std::string_view(it, 1);

    if (is_digit(*it) or (numberSign and *it == '-' and next_char and is_digit(next_char.value())))
    {
      auto double_val = to_double(std::string_view(it, expression.cend()));

      if (double_val)
      {
        const auto& [double_opt_val, processed_char_num] = *double_val;
        // parsing successful
        parsing.push_back(tokens::Number(double_opt_val, std::string_view(it, processed_char_num)));
        it += processed_char_num;

        openingParenthesis = value = numberSign = false;
        ope = canEnd = closingParenthesis = true;
      }
      else return tl::unexpected(ParsingError::wrong_format(tokens::Number(std::nan(""), char_v)));
    }
    else if (tokens::Operator::is_operator(*it))
    {
      if (ope)
      {
        parsing.push_back(tokens::Operator(char_v));

        openingParenthesis = value = true;
        ope = numberSign = closingParenthesis = canEnd = false;
        it++;
      }
      else return tl::unexpected(ParsingError::unexpected(tokens::Operator(char_v)));
    }
    else if (*it == '(')
    {
      if (openingParenthesis)
      {
        if (not parsing.empty() and std::holds_alternative<tokens::Function>(parsing.back()))
        {
          parsing.emplace_back(tokens::FunctionCallStart(char_v));
          last_opened_pth.push(FUNCTION_CALL_PTH);
        }
        else
        {
          parsing.emplace_back(tokens::OpeningParenthesis(char_v));
          last_opened_pth.push(NORMAL_PTH);
        }

        numberSign = value = openingParenthesis = true;
        ope = closingParenthesis = canEnd = false;
        it++;
      }
      else return tl::unexpected(ParsingError::unexpected(tokens::OpeningParenthesis(char_v)));
    }
    else if (*it == ')')
    {
      if (closingParenthesis and not last_opened_pth.empty())
      {
        if (last_opened_pth.top() == FUNCTION_CALL_PTH)
        {
          parsing.emplace_back(tokens::FunctionCallEnd(char_v));
        }
        else parsing.emplace_back(tokens::ClosingParenthesis(char_v));

        last_opened_pth.pop();

        ope = canEnd = closingParenthesis = true;
        value = numberSign = openingParenthesis = false;
        it++;
      }
      else return tl::unexpected(ParsingError::unexpected(tokens::ClosingParenthesis(char_v)));
    }
    else if (*it == ' ')
      // spaces are skipped
      it++;
    else if (is_argument_separator(*it))
    {
      if (last_opened_pth.top() == FUNCTION_CALL_PTH)
        parsing.emplace_back(tokens::FunctionArgumentSeparator(char_v));
      else return tl::unexpected(ParsingError::unexpected(tokens::FunctionArgumentSeparator(char_v)));

      openingParenthesis = numberSign = value = true;
      canEnd = ope = closingParenthesis = false;
      it++;
    }
    else
    {
      if (value)
      {
        // the only possibilities left are variables and functions
        // to differentiate between them, functions always are followed by an opening parenthesis
        // e.g. "cos("

        auto token_begin = it;
        while (it != expression.cend() and
               not is_seperator(*it) and
               not is_argument_separator(*it)) { it++; }

        std::string_view token_v(token_begin, it);

        if (it == expression.cend() or *it != '(')
        {
          // can only be a variable when we reach the end of the expression
          parsing.emplace_back(tokens::Variable(token_v));

          openingParenthesis = numberSign = value = false;
          canEnd = ope = closingParenthesis = true;
        }
        else
        {
          parsing.emplace_back(tokens::Function(token_v));

          canEnd = closingParenthesis = ope = numberSign = value = false;
          openingParenthesis = true;
        }
      }
      else return tl::unexpected(ParsingError::unexpected(tokens::Unkown(char_v)));
    }
  }

  if (not last_opened_pth.empty())
  {
    std::string_view last_char_v = std::string_view(it-1, 1);
    if (last_opened_pth.top() == FUNCTION_CALL_PTH)
      return tl::unexpected(ParsingError::missing(tokens::FunctionCallEnd(last_char_v)));
    else return tl::unexpected(ParsingError::missing(tokens::ClosingParenthesis(last_char_v)));
  }

  if (not canEnd)
    return tl::unexpected(ParsingError::unexpected(tokens::EndOfExpression()));

  return parsing;
}

}
