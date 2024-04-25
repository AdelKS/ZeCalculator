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
#include <zecalculator/test-utils/utils.h>

using namespace zc::parsing;

int main()
{
  using namespace boost::ut;

  "simple expression"_test = []()
  {
    std::string expression = "2+2*2";

    auto expect_node = tokenize(expression).and_then(make_ast{expression});

    expect(bool(expect_node)) << expect_node << fatal;

    AST expected_node = ast::node::BinaryOperator<'+'>(
      tokens::Text(expression, 0), tokens::Text("+", 1),
      {shared::node::Number(2.0, tokens::Text{"2", 0}),
       ast::node::BinaryOperator<'*'>(tokens::Text("2*2", 2), tokens::Text("*", 3),
                                       {shared::node::Number(2.0, tokens::Text{"2", 2}),
                                        shared::node::Number(2.0, tokens::Text{"2", 4})})});

    expect(*expect_node == expected_node);

    if (*expect_node != expected_node )
      std::cout << *expect_node << std::endl;

    expect(direct_dependencies(*expect_node).empty());

  };

  "double parentheses"_test = []()
  {
    std::string expression = "(2)*(2)";

    auto expect_node = tokenize(expression).and_then(make_ast{expression});

    expect(bool(expect_node)) << expect_node << fatal;

    AST expected_node = ast::node::BinaryOperator<'*'>(
      tokens::Text(expression, 0), tokens::Text("*", 3),
        {shared::node::Number(2.0, tokens::Text{"2", 0}),
         shared::node::Number(2.0, tokens::Text{"2", 0})});

    expect(*expect_node == expected_node);

    if (*expect_node != expected_node )
      std::cout << *expect_node << std::endl;

    expect(direct_dependencies(*expect_node).empty());

  };

  "function expression"_test = []()
  {
    std::string expression = "(cos(sin(x)+1))+1";

    auto expect_node = tokenize(expression).and_then(make_ast{expression, std::array{"x"}});

    expect(bool(expect_node)) << expect_node << fatal;

    AST expected_node = ast::node::BinaryOperator<'+'>(tokens::Text(expression, 0), tokens::Text("+", 15),
      {ast::node::Function(tokens::Text("cos(sin(x)+1)", 1), tokens::Text("cos", 1), tokens::Text("sin(x)+1", 13),
        {ast::node::BinaryOperator<'+'>(tokens::Text("cos(sin(x)+1", 1), tokens::Text("+", 11),
          {ast::node::Function(tokens::Text("sin(x)", 5), tokens::Text("cos", 5), tokens::Text("x", 9),
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

    auto simple_ast = tokenize(expression).and_then(make_ast{expression});

    expect(bool(simple_ast)) << simple_ast << fatal;

    // "x" is considered a variable for now
    expect(direct_dependencies(simple_ast.value())
           == zc::deps::Deps{{"cos", zc::deps::FUNCTION},
                             {"sin", zc::deps::FUNCTION},
                             {"x", zc::deps::VARIABLE}});

    auto expect_node = simple_ast.transform(mark_input_vars{std::array{"x"}});

    expect(bool(expect_node)) << expect_node << fatal;

    // "x" became an "input variable" and therefore not an external dependency anymore
    expect(direct_dependencies(expect_node.value())
           == zc::deps::Deps{{"cos", zc::deps::FUNCTION}, {"sin", zc::deps::FUNCTION}});

    AST expected_node =
    ast::node::BinaryOperator<'+'>(
      tokens::Text(expression, 0), tokens::Text("+", 13),
      {ast::node::BinaryOperator<'+'>(
        tokens::Text("cos(x)+sin(x)", 0), tokens::Text("+", 6),
        {ast::node::Function(tokens::Text("cos(x)", 0), tokens::Text("cos", 0), tokens::Text("x", 4),
          {shared::node::InputVariable(tokens::Text("x", 4), 0)}),
         ast::node::Function(tokens::Text("sin(x)", 7), tokens::Text("sin", 7), tokens::Text("x", 11),
          {shared::node::InputVariable(tokens::Text("x", 11), 0)})}),
       shared::node::Number(1.0, tokens::Text("1", 12))});

    if (*expect_node != expected_node)
      std::cout << *expect_node;

    expect(*expect_node == expected_node);

  };

  "direct dependencies"_test = []()
  {
    std::string expression = "(cos(sin(x)+1+w)/u(f(h(y))))+1";

    auto expect_node = tokenize(expression).and_then(make_ast{expression, std::array{"x"}});

    expect(bool(expect_node)) << expect_node << fatal;

    expect(direct_dependencies(expect_node.value())
           == zc::deps::Deps{{"cos", zc::deps::FUNCTION},
                             {"sin", zc::deps::FUNCTION},
                             {"w", zc::deps::VARIABLE},
                             {"u", zc::deps::FUNCTION},
                             {"f", zc::deps::FUNCTION},
                             {"h", zc::deps::FUNCTION},
                             {"y", zc::deps::VARIABLE},});
  };

  "AST creation speed"_test = []()
  {
    constexpr std::string_view static_expr = "2+ 3 -  cos(x) - 2 + 3 * 2.5343E+12-34234+2-4 * 34 / 634534           + 45.4E+2";
    constexpr size_t static_expr_size = static_expr.size();
    constexpr auto duration = nanoseconds(2s);

    constexpr size_t max_random_padding = 10;
    size_t dummy = 0;
    std::string expr(static_expr);
    expr.reserve(static_expr.size() + max_random_padding);

    size_t i = 0;
    size_t iterations = loop_call_for(duration, [&]{
      // resize with variable number of extra spaces
      // just to fool the compiler so it thinks each call to this function is unique
      i = (i + 1) % max_random_padding;
      expr.resize(static_expr_size + i, ' ');

      auto exp_ast = tokenize(expr).and_then(make_ast{expr});

      dummy += text_token(**exp_ast).substr.size();
    });

    // the absolute value doesn't mean anything really, but we can compare between performance improvements
    std::cout << "AST creation time: "
              << duration_cast<nanoseconds>(duration/iterations).count() << "ns"
              << std::endl;
    std::cout << "dummy: " << dummy << std::endl;

  };
}
