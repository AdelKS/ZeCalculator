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
#include <boost/ut.hpp>
#include <zecalculator/test-utils/print-utils.h>
#include <zecalculator/test-utils/structs.h>

using namespace zc::parsing;

int main()
{
  using namespace boost::ut;

  "simple expression"_test = []()
  {
    auto parsing = tokenize("2+2*2");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_uast(parsing.value());

    expect(bool(expect_node));

    UAST expected_node = uast::node::BinaryOperator<'+'>(
      1,
      {shared::node::Number(2.0, tokens::Text{"2", 0}),
       uast::node::BinaryOperator<'*'>(3,
                                       {shared::node::Number(2.0, tokens::Text{"2", 2}),
                                        shared::node::Number(2.0, tokens::Text{"2", 4})})});

    expect(*expect_node == expected_node);

    if (*expect_node != expected_node )
      std::cout << *expect_node << std::endl;

  };

  "function expression"_test = []()
  {
    auto parsing = tokenize("(cos(sin(x)+1))+1");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_uast(parsing.value(), zc::Vars<1>{"x"});

    expect(bool(expect_node));

    UAST expected_node = uast::node::BinaryOperator<'+'>(15,
      {uast::node::Function(tokens::Text("cos", 1),
        {uast::node::BinaryOperator<'+'>(11,
          {uast::node::Function(tokens::Text("sin", 5),
            {shared::node::InputVariable(tokens::Text("x", 9), 0)}),
             shared::node::Number(1.0, tokens::Text("1", 12))})}),
      shared::node::Number(1.0, tokens::Text("1", 16))});

    if (*expect_node != expected_node)
      std::cout << *expect_node;

    expect(*expect_node == expected_node);

  };
}