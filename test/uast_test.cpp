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
    std::string expression = "2+2*2";
    auto parsing = tokenize(expression);

    expect(bool(parsing)) << parsing;

    auto expect_node = make_uast(expression, parsing.value());

    expect(bool(expect_node));

    UAST expected_node = uast::node::BinaryOperator<'+'>(
      tokens::Text(expression, 0), tokens::Text("+", 1),
      {shared::node::Number(2.0, tokens::Text{"2", 0}),
       uast::node::BinaryOperator<'*'>(tokens::Text("2*2", 2), tokens::Text("*", 3),
                                       {shared::node::Number(2.0, tokens::Text{"2", 2}),
                                        shared::node::Number(2.0, tokens::Text{"2", 4})})});

    expect(*expect_node == expected_node);

    if (*expect_node != expected_node )
      std::cout << *expect_node << std::endl;

    expect(direct_dependencies(*expect_node).empty());

  };

  "function expression"_test = []()
  {
    std::string expression = "(cos(sin(x)+1))+1";
    auto parsing = tokenize("(cos(sin(x)+1))+1");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_uast(expression, parsing.value(), std::array{"x"});

    expect(bool(expect_node));

    UAST expected_node = uast::node::BinaryOperator<'+'>(tokens::Text(expression, 0), tokens::Text("+", 15),
      {uast::node::Function(tokens::Text("cos(sin(x)+1)", 1), tokens::Text("cos", 1), tokens::Text("sin(x)+1", 13),
        {uast::node::BinaryOperator<'+'>(tokens::Text("cos(sin(x)+1", 1), tokens::Text("+", 11),
          {uast::node::Function(tokens::Text("sin(x)", 5), tokens::Text("cos", 5), tokens::Text("x", 9),
            {shared::node::InputVariable(tokens::Text("x", 9), 0)}),
             shared::node::Number(1.0, tokens::Text("1", 12))})}),
      shared::node::Number(1.0, tokens::Text("1", 16))});

    if (*expect_node != expected_node)
      std::cout << *expect_node;

    expect(*expect_node == expected_node);

    expect(direct_dependencies(*expect_node)
           == zc::deps::Deps{{"cos", zc::deps::FUNCTION}, {"sin", zc::deps::FUNCTION}});

  };

  "mark input vars"_test = []()
  {
    std::string expression = "cos(x)+sin(x)+1";

    auto parsing = tokenize(expression);

    expect(bool(parsing)) << parsing;

    auto simple_uast = make_uast(expression, parsing.value());

    // "x" is considered a variable for now
    expect(direct_dependencies(simple_uast.value())
           == zc::deps::Deps{{"cos", zc::deps::FUNCTION},
                             {"sin", zc::deps::FUNCTION},
                             {"x", zc::deps::VARIABLE}});

    auto expect_node = simple_uast.transform(mark_input_vars{std::array{"x"}});

    expect(bool(expect_node));

    // "x" became an "input variable" and therefore not an external dependency anymore
    expect(direct_dependencies(expect_node.value())
           == zc::deps::Deps{{"cos", zc::deps::FUNCTION}, {"sin", zc::deps::FUNCTION}});

    UAST expected_node =
    uast::node::BinaryOperator<'+'>(
      tokens::Text(expression, 0), tokens::Text("+", 13),
      {uast::node::BinaryOperator<'+'>(
        tokens::Text("cos(x)+sin(x)", 0), tokens::Text("+", 6),
        {uast::node::Function(tokens::Text("cos(x)", 0), tokens::Text("cos", 0), tokens::Text("x", 4),
          {shared::node::InputVariable(tokens::Text("x", 4), 0)}),
         uast::node::Function(tokens::Text("sin(x)", 7), tokens::Text("sin", 7), tokens::Text("x", 11),
          {shared::node::InputVariable(tokens::Text("x", 11), 0)})}),
       shared::node::Number(1.0, tokens::Text("1", 12))});

    if (*expect_node != expected_node)
      std::cout << *expect_node;

    expect(*expect_node == expected_node);

  };

  "direct dependencies"_test = []()
  {
    std::string expression = "(cos(sin(x)+1+w)/u(f(h(y))))+1";

    auto parsing = tokenize(expression);

    expect(bool(parsing)) << parsing;

    auto expect_node = make_uast(expression, parsing.value(), std::array{"x"});

    expect(bool(expect_node));

    expect(direct_dependencies(expect_node.value())
           == zc::deps::Deps{{"cos", zc::deps::FUNCTION},
                             {"sin", zc::deps::FUNCTION},
                             {"w", zc::deps::VARIABLE},
                             {"u", zc::deps::FUNCTION},
                             {"f", zc::deps::FUNCTION},
                             {"h", zc::deps::FUNCTION},
                             {"y", zc::deps::VARIABLE},});
  };
}
