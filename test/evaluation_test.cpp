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

#include <zecalculator/utils/parser.h>
#include <zecalculator/utils/syntax_tree.h>
#include <zecalculator/utils/evaluation.h>

// testing specific headers
#include <boost/ut.hpp>
#include <zecalculator/test-utils/print-utils.h>

using namespace zc;

int main()
{
  using namespace boost::ut;

  "simple function evaluation"_test = []()
  {
    auto parsing = parse("cos(2)");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_tree(parsing.value());

    expect(bool(expect_node));

    expect(evaluate(expect_node.value()).value() == std::cos(2.0));
  };

  "simple expression evaluation"_test = []()
  {
    auto parsing = parse("2+2*2");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_tree(parsing.value());

    expect(bool(expect_node));

    expect(evaluate(expect_node.value()).value() == 6);
  };

  "complex expression evaluation"_test = []()
  {
    auto parsing = parse("2/3+2*2*exp(2)^2.5");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_tree(parsing.value());

    expect(bool(expect_node));

    const double res = evaluate(expect_node.value()).value();
    const double expected_res = 2./3.+2.*2.*std::pow(std::exp(2.), 2.5);

    expect(res == expected_res);
  };

  "global constant expression evaluation"_test = []()
  {
    auto parsing = parse("2*math::π + math::pi/2");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_tree(parsing.value());

    expect(bool(expect_node));

    const double res = evaluate(expect_node.value()).value();
    const double expected_res = 2.5 * std::numbers::pi;

    expect(res == expected_res);
  };

  "global constant registering and evaluation"_test = []()
  {
    MathWorld world;
    world.add_global_constant("my_constant1", 2.0);
    world.add_global_constant("my_constant2", 3.0);

    auto parsing = parse("my_constant1 + my_constant2");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_tree(parsing.value());

    expect(bool(expect_node));

    const double res = evaluate(expect_node.value(), world).value();
    const double expected_res = 5.0;

    expect(res == expected_res);
  };

  "global constant registering override"_test = []()
  {
    MathWorld world;
    world.add_global_constant("my_constant1", 2.0);
    world.add_global_constant("my_constant1", 3.0);

    auto parsing = parse("my_constant1");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_tree(parsing.value());

    expect(bool(expect_node)) << expect_node;

    const double res = evaluate(expect_node.value(), world).value();
    const double expected_res = 3.0;

    expect(res == expected_res);
  };

  "undefined global constant"_test = []()
  {
    auto parsing = parse("cos(1) + my_constant1");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_tree(parsing.value());

    expect(bool(expect_node)) << expect_node;

    const auto res = evaluate(expect_node.value());

    expect(not bool(res)) << res;
  };

  "input var evaluation shadowing a global constant"_test = []()
  {
    MathWorld world;
    world.add_global_constant("x", 2.0);
    auto parsing = parse("cos(x) + x");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_tree(parsing.value());

    expect(bool(expect_node)) << expect_node;

    const double res = evaluate(expect_node.value(), {{"x", 1.0}}, world).value();

    const double expected_res = std::cos(1.0) + 1.0;
    expect(res == expected_res);
  };
}
