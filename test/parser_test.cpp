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

  "simple expression"_test = []()
  {
    auto parsing = parse("2+2*2");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        tokens::Number(2., "2"),
        tokens::Operator("+"),
        tokens::Number(2., "2"),
        tokens::Operator("*"),
        tokens::Number(2., "2"),
    });

    expect(*parsing == expected_parsing);
  };

  "simple expression with spaces"_test = []()
  {
    auto parsing = parse("   2 +  2  *2");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        tokens::Number(2., "2"),
        tokens::Operator("+"),
        tokens::Number(2., "2"),
        tokens::Operator("*"),
        tokens::Number(2., "2"),
    });

    expect(*parsing == expected_parsing);
  };

  "function expression"_test = []()
  {
    auto parsing = parse("(cos(sin(x)+1))+1");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        tokens::OpeningParenthesis("("),
        tokens::Function("cos"),
        tokens::FunctionCallStart("("),
        tokens::Function("sin"),
        tokens::FunctionCallStart("("),
        tokens::Variable("x"),
        tokens::FunctionCallEnd(")"),
        tokens::Operator("+"),
        tokens::Number(1., "1"),
        tokens::FunctionCallEnd(")"),
        tokens::ClosingParenthesis(")"),
        tokens::Operator("+"),
        tokens::Number(1., "1"),
    });

    expect(*parsing == expected_parsing);
  };

  "two operators"_test = []()
  {
    auto parsing = parse("2*-1");

    expect(not parsing and
           parsing.error() == ParsingError::unexpected(tokens::Operator("-")))
        << parsing;
  };

  "extra parenthesis"_test = []()
  {
    auto parsing = parse("2+2)");

    expect(not parsing and
           parsing.error() == ParsingError::unexpected(tokens::ClosingParenthesis(")")));
  };

  "floating point operations"_test = []()
  {
    auto parsing = parse("223.231E+13+183.283E-132");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        tokens::Number(223.231E+13, "223.231E+13"),
        tokens::Operator("+"),
        tokens::Number(183.283E-132, "183.283E-132"),
    });

    expect(*parsing == expected_parsing);
  };

  "nested multi-variable functions"_test = []()
  {
    auto parsing = parse("f(1+g(x, r(h(x))), x)");

    expect(bool(parsing)) << parsing;

    auto expected_parsing = std::vector<Token>({
        tokens::Function("f"),
        tokens::FunctionCallStart("("),
        tokens::Number(1, "1"),
        tokens::Operator("+"),
        tokens::Function("g"),
        tokens::FunctionCallStart("("),
        tokens::Variable("x"),
        tokens::FunctionArgumentSeparator(","),
        tokens::Function("r"),
        tokens::FunctionCallStart("("),
        tokens::Function("h"),
        tokens::FunctionCallStart("("),
        tokens::Variable("x"),
        tokens::FunctionCallEnd(")"),
        tokens::FunctionCallEnd(")"),
        tokens::FunctionCallEnd(")"),
        tokens::FunctionArgumentSeparator(","),
        tokens::Variable("x"),
        tokens::FunctionCallEnd(")")
    });

    if(parsing)
      expect(*parsing == expected_parsing);
  };

  return 0;
}
