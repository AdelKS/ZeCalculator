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

#include "zecalculator/parsing/data_structures/decl/ast.h"
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

    auto expect_node
      = tokenize(expression).and_then(make_ast{expression}).transform(flatten_separators);

    expect(bool(expect_node)) << expect_node << fatal;

    AST expected_node = ast::Node::make_func(
      AST::Func::OP_ADD,
      tokens::Text{"+", 1},
      tokens::Text{expression, 0},
      {ast::Node::make_number(tokens::Text{"2", 0}, 2.0),
       ast::Node::make_func(AST::Func::OP_MULTIPLY,
                            tokens::Text{"*", 3},
                            tokens::Text{"2*2", 2},
                            {ast::Node::make_number(tokens::Text{"2", 2}, 2.0),
                             ast::Node::make_number(tokens::Text{"2", 4}, 2.0)})});

    expect(*expect_node == expected_node) << *expect_node;

    expect(direct_dependencies(*expect_node).empty());

  };

  "double parentheses"_test = []()
  {
    std::string expression = "(2)*(2)";

    auto expect_node = tokenize(expression).and_then(make_ast{expression});

    expect(bool(expect_node)) << expect_node << fatal;

    AST expected_node = AST::make_func(AST::Func::OP_MULTIPLY,
                                         tokens::Text{"*", 3},
                                         tokens::Text{expression, 0},
                                         {AST::make_number(tokens::Text{"2", 1}, 2.0),
                                          AST::make_number(tokens::Text{"2", 5}, 2.0)});

    expect(*expect_node == expected_node) << *expect_node;

    expect(direct_dependencies(*expect_node).empty());

  };

  "power & unary minus"_test = []()
  {
    std::string expression = "1^-cos(x)";

    auto expect_node = tokenize(expression).and_then(make_ast{expression});

    expect(bool(expect_node)) << expect_node << fatal;

    AST expected_node
      = AST::make_func(AST::Func::OP_POWER,
                       tokens::Text{"^", 1},
                       tokens::Text{expression, 0},
                       {AST::make_number(tokens::Text{"1", 0}, 1.),
                        AST::make_func(AST::Func::OP_UNARY_MINUS,
                                       tokens::Text{"-", 2},
                                       tokens::Text{"-cos(x)", 2},
                                       {AST::make_func(AST::Func::FUNCTION,
                                                       tokens::Text{"cos", 3},
                                                       tokens::Text{"cos(x)", 3},
                                                       {AST::make_var(tokens::Text{"x", 7})})})});

    expect(*expect_node == expected_node) << *expect_node;

  };

  "subtract & unary minus"_test = []()
  {
    std::string expression = "1--cos(x)";

    auto expect_node = tokenize(expression).and_then(make_ast{expression});

    expect(bool(expect_node)) << expect_node << fatal;

    AST expected_node
      = AST::make_func(AST::Func::OP_SUBTRACT,
                       tokens::Text{"-", 1},
                       tokens::Text{expression, 0},
                       {AST::make_number(tokens::Text{"1", 0}, 1.),
                        AST::make_func(AST::Func::OP_UNARY_MINUS,
                                       tokens::Text{"-", 2},
                                       tokens::Text{"-cos(x)", 2},
                                       {AST::make_func(AST::Func::FUNCTION,
                                                       tokens::Text{"cos", 3},
                                                       tokens::Text{"cos(x)", 3},
                                                       {AST::make_var(tokens::Text{"x", 7})})})});

    expect(*expect_node == expected_node) << *expect_node;

  };

  "function expression"_test = []()
  {
    std::string expression = "(cos(sin(x)+1))+1";

    auto expect_node = tokenize(expression).and_then(make_ast{expression, std::array{"x"}});

    expect(bool(expect_node)) << expect_node << fatal;

    AST expected_node = AST::make_func(
      AST::Func::OP_ADD,
      tokens::Text{"+", 15},
      tokens::Text{expression, 0},
      {AST::make_func(AST::Func::FUNCTION,
                        tokens::Text{"cos", 1},
                        tokens::Text{"cos(sin(x)+1)", 1},
                        {AST::make_func(AST::Func::OP_ADD,
                                          tokens::Text{"+", 11},
                                          tokens::Text{"sin(x)+1", 5},
                                          {AST::make_func(AST::Func::FUNCTION,
                                                            tokens::Text{"sin", 5},
                                                            tokens::Text{"sin(x)", 5},
                                                            {AST::make_input_var(tokens::Text{"x", 9},
                                                                                 0)}),
                                           AST::make_number(tokens::Text{"1", 12}, 1.0)})}),
       AST::make_number(tokens::Text{"1", 16}, 1.0)});

    expect(*expect_node == expected_node) << *expect_node;

    expect(direct_dependencies(*expect_node)
           == zc::deps::Deps{{"cos", {zc::deps::Dep::FUNCTION, {1}}},
                             {"sin", {zc::deps::Dep::FUNCTION, {5}}}});

  };

  "mark input vars"_test = []()
  {
    std::string expression = "cos(x)+sin(x)+1";

    auto simple_ast = tokenize(expression).and_then(make_ast{expression});

    expect(bool(simple_ast)) << simple_ast << fatal;

    // "x" is considered a variable for now
    expect(direct_dependencies(simple_ast.value())
           == zc::deps::Deps{{"cos", {zc::deps::Dep::FUNCTION, {0}}},
                             {"sin", {zc::deps::Dep::FUNCTION, {7}}},
                             {"x", {zc::deps::Dep::VARIABLE, {4, 11}}}});

    auto expect_node = simple_ast.transform(mark_input_vars{std::array{"x"}});

    expect(bool(expect_node)) << expect_node << fatal;

    // "x" became an "input variable" and therefore not an external dependency anymore
    expect(direct_dependencies(expect_node.value())
           == zc::deps::Deps{{"cos", {zc::deps::Dep::FUNCTION, {0}}},
                             {"sin", {zc::deps::Dep::FUNCTION, {7}}}});

    AST expected_node = AST::make_func(
      AST::Func::OP_ADD,
      tokens::Text{"+", 13},
      tokens::Text{expression, 0},
      {AST::make_func(AST::Func::OP_ADD,
                        tokens::Text{"+", 6},
                        tokens::Text{"cos(x)+sin(x)", 0},
                        {AST::make_func(AST::Func::FUNCTION,
                                          tokens::Text{"cos", 0},
                                          tokens::Text{"cos(x)", 0},
                                          {AST::make_input_var(tokens::Text{"x", 4}, 0)}),
                         AST::make_func(AST::Func::FUNCTION,
                                          tokens::Text{"sin", 7},
                                          tokens::Text{"sin(x)", 7},
                                          {AST::make_input_var(tokens::Text{"x", 11}, 0)})}),
       AST::make_number(tokens::Text{"1", 14}, 1.0)});

    expect(*expect_node == expected_node) << *expect_node;

  };

  "direct dependencies"_test = []()
  {
    std::string expression = "(cos(sin(x)+1+w)/u(f(h(y))))+1+cos(x)+f(y)+u(w)";

    auto expect_node = tokenize(expression).and_then(make_ast{expression, std::array{"x"}});

    expect(bool(expect_node)) << expect_node << fatal;

    expect(direct_dependencies(expect_node.value())
           == zc::deps::Deps{{"cos", {zc::deps::Dep::FUNCTION, {1, 31}}},
                             {"sin", {zc::deps::Dep::FUNCTION, {5}}},
                             {"w", {zc::deps::Dep::VARIABLE, {14, 45}}},
                             {"u", {zc::deps::Dep::FUNCTION, {17, 43}}},
                             {"f", {zc::deps::Dep::FUNCTION, {19, 38}}},
                             {"h", {zc::deps::Dep::FUNCTION, {21}}},
                             {"y", {zc::deps::Dep::VARIABLE, {23, 40}}},});
  };
}
