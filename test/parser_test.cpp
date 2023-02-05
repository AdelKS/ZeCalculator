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
#include <boost/ut.hpp>

using namespace zc;
using namespace zc::parsing;

int main()
{
  using namespace boost::ut;

  "simple expression"_test = []()
  {
    auto parsing = parse("2+2*2");

    expect(bool(parsing)) << (parsing ? "" : parsing.error().error_name());

    auto expected_parsing =
        std::vector<Token>({
          {Token::Type::NUMBER, 2.},
          {Token::Type::OPERATOR, Token::Operator::PLUS},
          {Token::Type::NUMBER, 2.},
          {Token::Type::OPERATOR, Token::Operator::MULTIPLY},
          {Token::Type::NUMBER, 2.},
        });

    expect(*parsing == expected_parsing);
  };

  "simple expression with spaces"_test = []()
  {
    auto parsing = parse("   2 +  2  *2");

    expect(bool(parsing)) << (parsing ? "" : parsing.error().error_name());

    auto expected_parsing =
        std::vector<Token>({
          {Token::Type::NUMBER, 2.},
          {Token::Type::OPERATOR, Token::Operator::PLUS},
          {Token::Type::NUMBER, 2.},
          {Token::Type::OPERATOR, Token::Operator::MULTIPLY},
          {Token::Type::NUMBER, 2.},
        });

    expect(*parsing == expected_parsing);
  };

  "function expression"_test = []()
  {
    auto parsing = parse("(cos(sin(x)+1))+1");

    expect(bool(parsing)) << (parsing ? "" : parsing.error().error_name());

    auto expected_parsing =
        std::vector<Token>({
          {Token::Type::OPENING_PARENTHESIS},
          {Token::Type::FUNCTION, "cos"},
          {Token::Type::FUNCTION_CALL_START},
          {Token::Type::FUNCTION, "sin"},
          {Token::Type::FUNCTION_CALL_START},
          {Token::Type::VARIABLE, "x"},
          {Token::Type::FUNCTION_CALL_END},
          {Token::Type::OPERATOR, '+'},
          {Token::Type::NUMBER, 1.},
          {Token::Type::FUNCTION_CALL_END},
          {Token::Type::CLOSING_PARENTHESIS},
          {Token::Type::OPERATOR, '+'},
          {Token::Type::NUMBER, 1.},
        });

    expect(*parsing == expected_parsing);
  };

  "two operators"_test = []()
  {
    auto parsing = parse("2*-1");

    expect(not parsing and parsing.error().type == Error::Type::UNEXPECTED_OPERATOR);
  };

  "extra parenthesis"_test = []()
  {
    auto parsing = parse("2+2)");

    expect(not parsing and parsing.error().type == Error::Type::UNEXPECTED_CLOSING_PARENTHESIS);
  };

  "floating point operations"_test = []()
  {
    auto parsing = parse("223.231E+13+183.283E-132");

    expect(bool(parsing)) << (parsing ? "" : parsing.error().error_name());

    auto expected_parsing =
        std::vector<Token>({
          {Token::Type::NUMBER, 223.231E+13},
          {Token::Type::OPERATOR, '+'},
          {Token::Type::NUMBER, 183.283E-132},
        });

    expect(*parsing == expected_parsing);
  };

  return 0;
}
