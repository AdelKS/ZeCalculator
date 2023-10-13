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

using namespace zc;
using namespace zc::parsing;

int main()
{
  using namespace boost::ut;

  "simple expression"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;

    auto parsing = parsing::tokenize("2+2*2");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_tree(parsing.value(), world);

    expect(bool(expect_node));

    Tree<type> expected_node = node::ast::CppFunction<type, 2>(
      tokens::Operator('+', 1),
      world.template get<CppFunction<2>>(tokens::Operator::name_of('+')),
      {node::Number(2.0, tokens::Text{"2", 0, 1}),
       node::ast::CppFunction<type, 2>(tokens::Operator('*', 3),
                                       world.template get<CppFunction<2>>(tokens::Operator::name_of('*')),
                                       {node::Number(2.0, tokens::Text{"2", 2, 1}),
                                        node::Number(2.0, tokens::Text{"2", 4, 1})})});

    expect(*expect_node == expected_node);

    if (*expect_node != expected_node )
      std::cout << **expect_node << std::endl;

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "function expression"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;
    MathWorld<type> world;

    auto parsing = tokenize("(cos(sin(x)+1))+1");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_tree(parsing.value(), world, Vars<1>{"x"});

    expect(bool(expect_node));

    Tree<type> expected_node = node::ast::CppFunction<type, 2>(
      tokens::Operator('+', 15),
      world.template get<CppFunction<2>>(tokens::Operator::name_of('+')),
      {node::ast::CppFunction<type, 1>(
         tokens::Text("cos", 1, 3),
         world.template get<CppFunction<1>>("cos"),
         {node::ast::CppFunction<type, 2>(
           tokens::Operator('+', 11),
           world.template get<CppFunction<2>>(tokens::Operator::name_of('+')),
           {node::ast::CppFunction<type, 1>(tokens::Text("sin", 5, 3),
                                            world.template get<CppFunction<1>>("sin"),
                                            {node::InputVariable(tokens::Text("x", 9, 1), 0)}),
            node::Number(1.0, tokens::Text("1", 12, 1))})}),
       node::Number(1.0, tokens::Text("1", 16, 1))});

    if (*expect_node != expected_node)
      std::cout << *expect_node;

    expect(*expect_node == expected_node);

  } | std::tuple<AST_TEST, RPN_TEST>{};
}
