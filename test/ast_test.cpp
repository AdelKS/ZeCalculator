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
    constexpr Type type = std::is_same_v<StructType, AST_TEST> ? Type::AST : Type::RPN;

    zc::MathWorld<type> world;
    std::string expression = "2+2*2";

    auto parsing = tokenize(expression);

    expect(bool(parsing)) << parsing;

    auto expect_node = make_uast(expression, parsing.value()).and_then(bind<type>{expression, world});

    expect(bool(expect_node));

    AST<type> expected_node = ast::node::BinaryOperator<type, '+'>(
      tokens::Text{expression, 0},
      {shared::node::Number(2.0, tokens::Text{"2", 0}),
       ast::node::BinaryOperator<type, '*'>(tokens::Text{"2*2", 2},
                                            {shared::node::Number(2.0, tokens::Text{"2", 2}),
                                             shared::node::Number(2.0, tokens::Text{"2", 4})})});

    expect(*expect_node == expected_node);

    if (*expect_node != expected_node )
      std::cout << **expect_node << std::endl;

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "function expression"_test = []<class StructType>()
  {
    constexpr Type type = std::is_same_v<StructType, AST_TEST> ? Type::AST : Type::RPN;
    zc::MathWorld<type> world;

    std::string expression = "(cos(sin(x)+1))+1";

    auto parsing = tokenize(expression);

    expect(bool(parsing)) << parsing;

    auto expect_node = make_uast(expression, parsing.value(), zc::Vars<1>{"x"}).and_then(bind<type>{expression, world});

    expect(bool(expect_node));

    AST<type> expected_node = ast::node::BinaryOperator<type, '+'>(
      tokens::Text(expression, 0 ),
      {ast::node::CppFunction<type, 1>(
         tokens::Text("cos", 1),
         world.template get<zc::CppFunction<type, 1>>("cos"),
         {ast::node::BinaryOperator<type, '+'>(
           tokens::Text("sin(x)+1", 5),
           {ast::node::CppFunction<type, 1>(tokens::Text("sin", 5),
                                            world.template get<zc::CppFunction<type, 1>>("sin"),
                                            {shared::node::InputVariable(tokens::Text("x", 9), 0)}),
            shared::node::Number(1.0, tokens::Text("1", 12))})}),
       shared::node::Number(1.0, tokens::Text("1", 16))});

    if (*expect_node != expected_node)
      std::cout << *expect_node;

    expect(*expect_node == expected_node);

  } | std::tuple<AST_TEST, RPN_TEST>{};
}
