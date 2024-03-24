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

#include <zecalculator/parsing/parser.h>

// testing specific headers
#include <boost/ut.hpp>
#include <zecalculator/test-utils/print-utils.h>

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
           and parsing.error() == Error::unexpected(tokens::EndOfExpression("", SubstrInfo{8, 0}), expression))
      << parsing;
  };

  "signed positive number"_test = []()
  {
    auto parsing = tokenize("+12.2E+3");

    expect(bool(parsing) and parsing->size() == 1
           and std::holds_alternative<tokens::Number>(parsing->front())
           and std::get<tokens::Number>(parsing->front()).value == 12.2E+3)
      << parsing;
  };

  "signed negative number"_test = []()
  {
    auto parsing = tokenize("-12.2E+3");

    expect(bool(parsing) and parsing->size() == 1
           and std::holds_alternative<tokens::Number>(parsing->front())
           and std::get<tokens::Number>(parsing->front()).value == -12.2E+3)
      << parsing;
  };

  "equal sign expression"_test = []()
  {
    // the expression doesn't mean anything but can be properly tokenized
    auto parsing = tokenize("2+2=2");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        tokens::Number(2., tokens::Text{"2", 0}),
        tokens::BinaryOperator<'+'>(1),
        tokens::Number(2., tokens::Text{"2", 2}),
        tokens::BinaryOperator<'='>(3),
        tokens::Number(2., tokens::Text{"2", 4}),
    });

    expect(*parsing == expected_parsing);
  };

  "simple expression"_test = []()
  {
    auto parsing = tokenize("2+2*2");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        tokens::Number(2., tokens::Text{"2", 0}),
        tokens::BinaryOperator<'+'>(1),
        tokens::Number(2., tokens::Text{"2", 2}),
        tokens::BinaryOperator<'*'>(3),
        tokens::Number(2., tokens::Text{"2", 4}),
    });

    expect(*parsing == expected_parsing);
  };

  "simple expression with spaces"_test = []()
  {
    auto parsing = tokenize("   2 +  2  *2");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        tokens::Number(2., tokens::Text{"2", 3}),
        tokens::BinaryOperator<'+'>(5),
        tokens::Number(2., tokens::Text{"2", 8}),
        tokens::BinaryOperator<'*'>(11),
        tokens::Number(2., tokens::Text{"2", 12}),
    });

    expect(*parsing == expected_parsing);
  };

  "function expression"_test = []()
  {
    auto parsing = tokenize("(cos(sin(x)+1))+1");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        tokens::OpeningParenthesis("(", 0),
        tokens::Function("cos", 1),
        tokens::FunctionCallStart("(", 4),
        tokens::Function("sin", 5),
        tokens::FunctionCallStart("(", 8),
        tokens::Variable("x", 9),
        tokens::FunctionCallEnd(")", 10),
        tokens::BinaryOperator<'+'>(11),
        tokens::Number(1., tokens::Text{"1", 12}),
        tokens::FunctionCallEnd(")", 13),
        tokens::ClosingParenthesis(")", 14),
        tokens::BinaryOperator<'+'>(15),
        tokens::Number(1., tokens::Text{"1", 16}),
    });

    expect(*parsing == expected_parsing);
  };

  "two operators"_test = []()
  {
    std::string expression = "2*-1";
    auto parsing = tokenize(expression);

    expect(not parsing and
           parsing.error() == Error::unexpected(tokens::BinaryOperator<'-'>(2), expression))
        << parsing;
  };

  "extra parenthesis"_test = []()
  {
    std::string expression = "2+2)";
    auto parsing = tokenize(expression);

    expect(not parsing and
           parsing.error() == Error::unexpected(tokens::ClosingParenthesis(")", 3, 1), expression));
  };

  "floating point operations"_test = []()
  {
    auto parsing = tokenize("223.231E+13+183.283E-132");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        tokens::Number(223.231E+13, tokens::Text{"223.231E+13", 0}),
        tokens::BinaryOperator<'+'>(11),
        tokens::Number(183.283E-132, tokens::Text{"183.283E-132", 12}),
    });

    expect(*parsing == expected_parsing);
  };

  "nested multi-variable functions"_test = []()
  {
    auto parsing = tokenize("f(1+g(x, r(h(x))), x)");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        tokens::Function("f", 0),
        tokens::FunctionCallStart("(", 1),
        tokens::Number(1, tokens::Text{"1", 2}),
        tokens::BinaryOperator<'+'>(3),
        tokens::Function("g", 4),
        tokens::FunctionCallStart("(", 5),
        tokens::Variable("x", 6),
        tokens::FunctionArgumentSeparator(",", 7),
        tokens::Function("r", 9),
        tokens::FunctionCallStart("(", 10),
        tokens::Function("h", 11),
        tokens::FunctionCallStart("(", 12),
        tokens::Variable("x", 13),
        tokens::FunctionCallEnd(")", 14),
        tokens::FunctionCallEnd(")", 15),
        tokens::FunctionCallEnd(")", 16),
        tokens::FunctionArgumentSeparator(",", 17),
        tokens::Variable("x", 19),
        tokens::FunctionCallEnd(")", 20)
    });

    if(parsing)
      expect(*parsing == expected_parsing);
  };

  "SubstrInfo"_test = []()
  {
    static constexpr std::string_view str = "2+cos(3)";
    auto parsing = tokenize(str);

    expect(bool(parsing)) << parsing;

    const auto substrinfo = substr_info(parsing.value()[2]);

    expect(substrinfo.value().substr(str) == "cos");
    expect(substrinfo.value().substr_before(str) == "2+");
    expect(substrinfo.value().substr_after(str) == "(3)");
  };

  "missing function closing pth"_test = []()
  {
    static constexpr std::string_view str = "2+cos(3";
    auto parsing = tokenize(str);

    expect(not bool(parsing)) << parsing;

    expect(parsing.error() == Error::missing(tokens::FunctionCallEnd("", 7), std::string(str))) << parsing.error();
  };

  "missing normal closing pth"_test = []()
  {
    static constexpr std::string_view str = "(2+cos(3)";
    auto parsing = tokenize(str);

    expect(not bool(parsing)) << parsing;

    expect(parsing.error() == Error::missing(tokens::ClosingParenthesis("", 9), std::string(str))) << parsing.error();
  };

  "unexpected end of expression"_test = []()
  {
    static constexpr std::string_view str = "2+";
    auto parsing = tokenize(str);

    expect(not bool(parsing)) << parsing;

    expect(parsing.error() == Error::unexpected(tokens::EndOfExpression("", 2), std::string(str)))
      << parsing.error();
  };

  return 0;
}
