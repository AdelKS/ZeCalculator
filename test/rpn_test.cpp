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

using namespace zc::parsing;

int main()
{
  using namespace boost::ut;

  "simple rpn expression"_test = []()
  {
    std::string expression = "2 - 3 + 2";

    zc::MathWorld<Type::RPN> world;

    auto expect_tree = tokenize(expression)
                         .and_then(make_ast{expression})
                         .and_then(make_fast<Type::RPN>{expression, world});

    expect(bool(expect_tree)) << expect_tree << fatal;

    auto rpn_expr = make_RPN(expect_tree.value());

    RPN expected_rpn;
    expected_rpn.push_back(shared::node::Number{2.0});
    expected_rpn.push_back(shared::node::Number{3.0});
    expected_rpn.push_back(shared::node::Subtract{});
    expected_rpn.push_back(shared::node::Number{2.0});
    expected_rpn.push_back(shared::node::Add{});

    expect(bool(rpn_expr == expected_rpn)) << "Expected: " << expected_rpn << "Answer: " << rpn_expr;
  };
}
