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

  "simple expression"_test = []<class StructType>()
  {
    constexpr Type type = std::is_same_v<StructType, FAST_TEST> ? Type::FAST : Type::RPN;

    zc::MathWorld<type> world;
    std::string expression = "2+2*2";

    auto expect_node = tokenize(expression)
                         .and_then(make_ast{expression})
                         .and_then(make_fast<type>{expression, world});

    expect(bool(expect_node)) << expect_node << fatal;

    FAST<type> expected_node = fast::node::BinaryOperator<type, '+'>(
      tokens::Text{expression, 0},
      {shared::node::Number(2.0, tokens::Text{"2", 0}),
       fast::node::BinaryOperator<type, '*'>(tokens::Text{"2*2", 2},
                                            {shared::node::Number(2.0, tokens::Text{"2", 2}),
                                             shared::node::Number(2.0, tokens::Text{"2", 4})})});

    expect(*expect_node == expected_node);

    if (*expect_node != expected_node )
      std::cout << **expect_node << std::endl;

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "function expression"_test = []<class StructType>()
  {
    constexpr Type type = std::is_same_v<StructType, FAST_TEST> ? Type::FAST : Type::RPN;
    zc::MathWorld<type> world;

    std::string expression = "(cos(sin(x)+1))+1";

    auto expect_node = tokenize(expression)
                         .and_then(make_ast{expression, std::array{"x"}})
                         .and_then(make_fast<type>{expression, world});

    expect(bool(expect_node)) << expect_node << fatal;

    FAST<type> expected_node = fast::node::BinaryOperator<type, '+'>(
      tokens::Text(expression, 0 ),
      {fast::node::CppFunction<type, 1>(
         tokens::Text("cos", 1),
         world.template get<zc::CppFunction<type, 1>>("cos"),
         {fast::node::BinaryOperator<type, '+'>(
           tokens::Text("sin(x)+1", 5),
           {fast::node::CppFunction<type, 1>(tokens::Text("sin", 5),
                                            world.template get<zc::CppFunction<type, 1>>("sin"),
                                            {shared::node::InputVariable(tokens::Text("x", 9), 0)}),
            shared::node::Number(1.0, tokens::Text("1", 12))})}),
       shared::node::Number(1.0, tokens::Text("1", 16))});

    if (*expect_node != expected_node)
      std::cout << *expect_node;

    expect(*expect_node == expected_node);

  } | std::tuple<FAST_TEST, RPN_TEST>{};
}
