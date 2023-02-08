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

    auto expected_parsing =
        std::vector<Token>({
          {Token::NUMBER,   "2", 2.},
          {Token::OPERATOR, "+", '+'},
          {Token::NUMBER,   "2", 2.},
          {Token::OPERATOR, "*", '*'},
          {Token::NUMBER,   "2", 2.},
          {Token::END_OF_EXPRESSION},
        });

    expect(*parsing == expected_parsing);
  };

  "simple expression with spaces"_test = []()
  {
    auto parsing = parse("   2 +  2  *2");

    expect(bool(parsing)) << parsing;

    auto expected_parsing =
        std::vector<Token>({
          {Token::NUMBER,   "2", 2.},
          {Token::OPERATOR, "+", '+'},
          {Token::NUMBER,   "2", 2.},
          {Token::OPERATOR, "*", '*'},
          {Token::NUMBER,   "2", 2.},
          {Token::END_OF_EXPRESSION},
        });

    expect(*parsing == expected_parsing);
  };

  "function expression"_test = []()
  {
    auto parsing = parse("(cos(sin(x)+1))+1");

    expect(bool(parsing)) << parsing;

    auto expected_parsing =
        std::vector<Token>({
          {Token::OPENING_PARENTHESIS, "("},
          {Token::FUNCTION, "cos"},
          {Token::FUNCTION_CALL_START, "("},
          {Token::FUNCTION, "sin"},
          {Token::FUNCTION_CALL_START, "("},
          {Token::VARIABLE, "x"},
          {Token::FUNCTION_CALL_END, ")"},
          {Token::OPERATOR, "+", '+'},
          {Token::NUMBER, "1", 1.},
          {Token::FUNCTION_CALL_END, ")"},
          {Token::CLOSING_PARENTHESIS, ")"},
          {Token::OPERATOR, "+", '+'},
          {Token::NUMBER, "1", 1.},
          {Token::END_OF_EXPRESSION},
        });

    expect(*parsing == expected_parsing);
  };

  "two operators"_test = []()
  {
    auto parsing = parse("2*-1");

    expect(not parsing and
           parsing.error().error_type == Error::UNEXPECTED and
           parsing.error().token_type == Token::OPERATOR and
           parsing.error().where == "-") << parsing;
  };

  "extra parenthesis"_test = []()
  {
    auto parsing = parse("2+2)");

    expect(not parsing and
           parsing.error().error_type == Error::UNEXPECTED and
           parsing.error().token_type == Token::CLOSING_PARENTHESIS);
  };

  "floating point operations"_test = []()
  {
    auto parsing = parse("223.231E+13+183.283E-132");

    expect(bool(parsing)) << parsing;

    auto expected_parsing =
        std::vector<Token>({
          {Token::NUMBER, "223.231E+13", 223.231E+13},
          {Token::OPERATOR, "+", '+'},
          {Token::NUMBER, "183.283E-132", 183.283E-132},
          {Token::END_OF_EXPRESSION},
        });

    expect(*parsing == expected_parsing);
  };

  return 0;
}
