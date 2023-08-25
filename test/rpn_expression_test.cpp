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

#include <zecalculator/zecalculator.h>

// testing specific headers
#include <zecalculator/test-utils/print-utils.h>
#include <boost/ut.hpp>
#include <chrono>
#include <zecalculator/parsing/data_structures/rpn_expression.h>

using namespace std::chrono;

using namespace zc;
using namespace zc::parsing;

int main()
{
  using namespace boost::ut;

  "simple rpn expression"_test = []()
  {
   auto parsing = parse("2 - 3 + 2");

    expect(bool(parsing)) << parsing;

    auto expect_tree = make_tree(parsing.value());

    expect(bool(expect_tree));

    auto rpn_expr = make_rpn_expression(expect_tree.value());

    rpn_expression expected_rpn_expr;
    expected_rpn_expr.push_back(tokens::Number(2.0, tokens::Text{"2", 0, 1}));
    expected_rpn_expr.push_back(tokens::Number(3.0, tokens::Text{"3", 4, 1}));
    expected_rpn_expr.push_back(tokens::Operator('-', 2));
    expected_rpn_expr.push_back(tokens::Number(2.0, tokens::Text{"2", 8, 1}));
    expected_rpn_expr.push_back(tokens::Operator('+', 6));

    expect(bool(rpn_expr == expected_rpn_expr)) << "Expected: " << expected_rpn_expr << "Answer: " << rpn_expr;
  };

}
