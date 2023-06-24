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

#include <zecalculator/utils/parser.h>

// testing specific headers
#include <boost/ut.hpp>
#include <zecalculator/test-utils/print-utils.h>

using namespace zc;

int main()
{
  using namespace boost::ut;

  "empty expression"_test = []()
  {
    auto parsing = parse("        ");

    expect(not bool(parsing)
           and parsing.error() == ParsingError::unexpected(tokens::EndOfExpression()))
      << parsing;
  };

  "simple expression"_test = []()
  {
    auto parsing = parse("2+2*2");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        tokens::Number(2., tokens::Text{"2", 0, 1}),
        tokens::Operator('+', 1),
        tokens::Number(2., tokens::Text{"2", 2, 1}),
        tokens::Operator('*', 3),
        tokens::Number(2., tokens::Text{"2", 4, 1}),
    });

    expect(*parsing == expected_parsing);
  };

  "simple expression with spaces"_test = []()
  {
    auto parsing = parse("   2 +  2  *2");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        tokens::Number(2., tokens::Text{"2", 3, 1}),
        tokens::Operator('+', 5),
        tokens::Number(2., tokens::Text{"2", 8, 1}),
        tokens::Operator('*', 11),
        tokens::Number(2., tokens::Text{"2", 12, 1}),
    });

    expect(*parsing == expected_parsing);
  };

  "function expression"_test = []()
  {
    auto parsing = parse("(cos(sin(x)+1))+1");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        tokens::OpeningParenthesis("(", 0, 1),
        tokens::Function("cos", 1, 3),
        tokens::FunctionCallStart("(", 4, 1),
        tokens::Function("sin", 5, 3),
        tokens::FunctionCallStart("(", 8, 1),
        tokens::Variable("x", 9, 1),
        tokens::FunctionCallEnd(")", 10, 1),
        tokens::Operator('+', 11),
        tokens::Number(1., tokens::Text{"1", 12, 1}),
        tokens::FunctionCallEnd(")", 13, 1),
        tokens::ClosingParenthesis(")", 14, 1),
        tokens::Operator('+', 15),
        tokens::Number(1., tokens::Text{"1", 16, 1}),
    });

    expect(*parsing == expected_parsing);
  };

  "two operators"_test = []()
  {
    auto parsing = parse("2*-1");

    expect(not parsing and
           parsing.error() == ParsingError::unexpected(tokens::Operator('-', 2)))
        << parsing;
  };

  "extra parenthesis"_test = []()
  {
    auto parsing = parse("2+2)");

    expect(not parsing and
           parsing.error() == ParsingError::unexpected(tokens::ClosingParenthesis(")", 3, 1)));
  };

  "floating point operations"_test = []()
  {
    auto parsing = parse("223.231E+13+183.283E-132");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        tokens::Number(223.231E+13, tokens::Text{"223.231E+13", 0, 11}),
        tokens::Operator('+', 11),
        tokens::Number(183.283E-132, tokens::Text{"183.283E-132", 12, 12}),
    });

    expect(*parsing == expected_parsing);
  };

  "nested multi-variable functions"_test = []()
  {
    auto parsing = parse("f(1+g(x, r(h(x))), x)");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        tokens::Function("f", 0, 1),
        tokens::FunctionCallStart("(", 1, 1),
        tokens::Number(1, tokens::Text{"1", 2, 1}),
        tokens::Operator('+', 3),
        tokens::Function("g", 4, 1),
        tokens::FunctionCallStart("(", 5, 1),
        tokens::Variable("x", 6, 1),
        tokens::FunctionArgumentSeparator(",", 7, 1),
        tokens::Function("r", 9, 1),
        tokens::FunctionCallStart("(", 10, 1),
        tokens::Function("h", 11, 1),
        tokens::FunctionCallStart("(", 12, 1),
        tokens::Variable("x", 13, 1),
        tokens::FunctionCallEnd(")", 14, 1),
        tokens::FunctionCallEnd(")", 15, 1),
        tokens::FunctionCallEnd(")", 16, 1),
        tokens::FunctionArgumentSeparator(",", 17, 1),
        tokens::Variable("x", 19, 1),
        tokens::FunctionCallEnd(")", 20, 1)
    });

    if(parsing)
      expect(*parsing == expected_parsing);
  };

  "SubstrInfo"_test = []()
  {
    static constexpr std::string_view str = "2+cos(3)";
    auto parsing = parse(str);

    expect(bool(parsing)) << parsing;

    const auto substrinfo = substr_info(parsing.value()[2]);

    expect(substrinfo.substr(str) == "cos");
    expect(substrinfo.substr_before(str) == "2+");
    expect(substrinfo.substr_after(str) == "(3)");
  };

  return 0;
}
