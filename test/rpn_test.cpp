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
#include <zecalculator/parsing/data_structures/rpn.h>

using namespace zc;
using namespace zc::parsing;

int main()
{
  using namespace boost::ut;

  "simple rpn expression"_test = []()
  {
    auto parsing = tokenize("2 - 3 + 2");

    expect(bool(parsing)) << parsing;

    MathWorld<parsing::Type::RPN> world;

    auto expect_tree = make_tree(parsing.value(), world);

    expect(bool(expect_tree));

    auto rpn_expr = parsing::make_RPN(expect_tree.value());

    RPN expected_rpn;
    expected_rpn.push_back(node::Number(2.0, tokens::Text{"2", 0}));
    expected_rpn.push_back(node::Number(3.0, tokens::Text{"3", 4}));
    expected_rpn.push_back(node::rpn::Operator<'-', 2>(2));
    expected_rpn.push_back(tokens::Number(2.0, tokens::Text{"2", 8}));
    expected_rpn.push_back(node::rpn::Operator<'+', 2>(6));

    expect(bool(rpn_expr == expected_rpn)) << "Expected: " << expected_rpn << "Answer: " << rpn_expr;
  };

}
