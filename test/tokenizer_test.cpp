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

#include "zecalculator/parsing/data_structures/token.h"
#include <zecalculator/parsing/parser.h>

// testing specific headers
#include <boost/ut.hpp>
#include <zecalculator/test-utils/print-utils.h>
#include <zecalculator/test-utils/utils.h>

using namespace zc;
using namespace zc::parsing;

int main()
{
  using namespace boost::ut;

  "empty expression"_test = []()
  {
    std::string expression = "        ";
    auto parsing = tokenize(expression);

    expect(not bool(parsing)
           and parsing.error()
                 == Error::unexpected(Token(tokens::END_OF_EXPRESSION,
                                            tokens::Text{"", 8}),
                                      expression))
      << parsing;
  };

  "signed positive number"_test = []()
  {
    auto parsing = tokenize("+12.2E+3");

    expect(bool(parsing) and parsing->size() == 1
           and parsing->front().type == tokens::NUMBER
           and parsing->front().value == 12.2E+3)
      << parsing;
  };

  "signed negative number"_test = []()
  {
    auto parsing = tokenize("-12.2E+3");

    expect(bool(parsing) and parsing->size() == 1
           and parsing->front().type == tokens::NUMBER
           and parsing->front().value == -12.2E+3)
      << parsing;
  };

  "equal sign expression"_test = []()
  {
    // the expression doesn't mean anything but can be properly tokenized
    auto parsing = tokenize("2+2=2");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        Token::Number(2., "2", 0),
        Token::Add("+", 1),
        Token::Number(2., "2", 2),
        Token::Assign("=", 3),
        Token::Number(2., "2", 4),
    });

    expect(*parsing == expected_parsing) << *parsing;
  };

  "simple expression"_test = []()
  {
    auto parsing = tokenize("2+2*2");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        Token::Number(2., "2", 0),
        Token::Add("+", 1),
        Token::Number(2., "2", 2),
        Token::Multiply("*", 3),
        Token::Number(2., "2", 4),
    });

    expect(*parsing == expected_parsing);
  };

  "simple expression with spaces"_test = []()
  {
    auto parsing = tokenize("   2 +  2  *2");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        Token::Number(2., "2", 3),
        Token::Add("+", 5),
        Token::Number(2., "2", 8),
        Token::Multiply("*", 11),
        Token::Number(2., "2", 12),
    });

    expect(*parsing == expected_parsing);
  };

  "function expression"_test = []()
  {
    auto parsing = tokenize("(cos(sin(x)+1))+1");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        Token::OpeningParenthesis("(", 0),
        Token::Function("cos", 1),
        Token::FunctionCallStart("(", 4),
        Token::Function("sin", 5),
        Token::FunctionCallStart("(", 8),
        Token::Variable("x", 9),
        Token::FunctionCallEnd(")", 10),
        Token::Add("+", 11),
        Token::Number(1., "1", 12),
        Token::FunctionCallEnd(")", 13),
        Token::ClosingParenthesis(")", 14),
        Token::Add("+", 15),
        Token::Number(1., "1", 16),
    });

    expect(*parsing == expected_parsing);
  };

  "two operators"_test = []()
  {
    std::string expression = "2*-1";
    auto parsing = tokenize(expression);

    expect(not parsing and
           parsing.error() == Error::unexpected(Token::Subtract("-", 2), expression))
        << parsing;
  };

  "extra parenthesis"_test = []()
  {
    std::string expression = "2+2)";
    auto parsing = tokenize(expression);

    expect(not parsing and
           parsing.error() == Error::unexpected(Token::ClosingParenthesis(")", 3), expression));
  };

  "floating point operations"_test = []()
  {
    auto parsing = tokenize("223.231E+13+183.283E-132");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        Token::Number(223.231E+13, "223.231E+13", 0),
        Token::Add("+", 11),
        Token::Number(183.283E-132, "183.283E-132", 12),
    });

    expect(*parsing == expected_parsing);
  };

  "nested multi-variable functions"_test = []()
  {
    auto parsing = tokenize("f(1+g(x, r(h(x))), x)");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        Token::Function("f", 0),
        Token::FunctionCallStart("(", 1),
        Token::Number(1, "1", 2),
        Token::Add("+", 3),
        Token::Function("g", 4),
        Token::FunctionCallStart("(", 5),
        Token::Variable("x", 6),
        Token::Separator(",", 7),
        Token::Function("r", 9),
        Token::FunctionCallStart("(", 10),
        Token::Function("h", 11),
        Token::FunctionCallStart("(", 12),
        Token::Variable("x", 13),
        Token::FunctionCallEnd(")", 14),
        Token::FunctionCallEnd(")", 15),
        Token::FunctionCallEnd(")", 16),
        Token::Separator(",", 17),
        Token::Variable("x", 19),
        Token::FunctionCallEnd(")", 20)
    });

    if(parsing)
      expect(*parsing == expected_parsing);
  };

  "Token data"_test = []()
  {
    static constexpr std::string_view str = "2+cos(3)";
    auto parsing = tokenize(str);

    expect(bool(parsing)) << [&]{ return parsing.error(); } << fatal;

    const auto& token = parsing.value()[2];

    expect(token.substr == "cos");
    expect(token.begin == 2);
  };

  "missing function closing pth"_test = []()
  {
    static constexpr std::string_view str = "2+cos(3";
    auto parsing = tokenize(str);

    expect(not bool(parsing)) << parsing;

    expect(parsing.error() == Error::missing(Token::FunctionCallEnd("", 7), std::string(str))) << parsing.error();
  };

  "missing normal closing pth"_test = []()
  {
    static constexpr std::string_view str = "(2+cos(3)";
    auto parsing = tokenize(str);

    expect(not bool(parsing)) << parsing;

    expect(parsing.error() == Error::missing(Token::ClosingParenthesis("", 9), std::string(str))) << parsing.error();
  };

  "unexpected end of expression"_test = []()
  {
    static constexpr std::string_view str = "2+";
    auto parsing = tokenize(str);

    expect(not bool(parsing)) << parsing;

    expect(parsing.error() == Error::unexpected(Token::EndOfExpression(2), std::string(str)))
      << parsing.error();
  };

  "tokenization speed"_test = []()
  {
    constexpr std::string_view static_expr = "2+ 3 -  cos(x) - 2 + 3 * 2.5343E+12-34234+2-4 * 34 / 634534           + 45.4E+2";
    constexpr size_t static_expr_size = static_expr.size();
    constexpr auto duration = nanoseconds(500ms);

    constexpr size_t max_random_padding = 10;
    size_t dummy = 0;
    std::string expr(static_expr);
    expr.reserve(static_expr.size() + max_random_padding);

    size_t i = 0;
    size_t iterations = loop_call_for(duration, [&]{
      // resize with variable number of extra spaces
      // just to fool the compiler so it thinks each call to this function is unique
      i = (i + 1) % max_random_padding;
      expr.resize(static_expr_size + i, ' ');

      auto exp_parsing = tokenize(expr);
      dummy += exp_parsing.value().back().substr.size();
    });

    // the absolute value doesn't mean anything really, but we can compare between performance improvements
    std::cout << "tokenization time: "
              << duration_cast<nanoseconds>(duration/iterations).count() << "ns"
              << std::endl;
    std::cout << "dummy: " << dummy << std::endl;

  };

  return 0;
}
