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

using namespace zc;

int main()
{
  using namespace boost::ut;

  "simple function evaluation"_test = []()
  {
    MathWorld world;

    auto parsing = parse("cos(2)");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_tree(parsing.value());

    expect(bool(expect_node));

    expect(evaluate(expect_node.value(), world).value() == std::cos(2.0));
  };

  "simple expression evaluation"_test = []()
  {
    MathWorld world;

    auto parsing = parse("2+2*2");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_tree(parsing.value());

    expect(bool(expect_node));

    expect(evaluate(expect_node.value(), world).value() == 6);
  };

  "complex expression evaluation"_test = []()
  {
    MathWorld world;

    auto parsing = parse("2/3+2*2*exp(2)^2.5");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_tree(parsing.value());

    expect(bool(expect_node));

    const double res = evaluate(expect_node.value(), world).value();
    const double expected_res = 2./3.+2.*2.*std::pow(std::exp(2.), 2.5);

    expect(res == expected_res);
  };

  "global constant expression evaluation"_test = []()
  {
    MathWorld world;

    auto parsing = parse("2*math::π + math::pi/2");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_tree(parsing.value());

    expect(bool(expect_node));

    const double res = evaluate(expect_node.value(), world).value();
    const double expected_res = 2.5 * std::numbers::pi;

    expect(res == expected_res);
  };

  "global constant registering and evaluation"_test = []()
  {
    MathWorld world;
    world.add<GlobalConstant>("my_constant1", 2.0);
    world.add<GlobalConstant>("my_constant2", 3.0);

    auto parsing = parse("my_constant1 + my_constant2");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_tree(parsing.value());

    expect(bool(expect_node));

    const double res = evaluate(expect_node.value(), world).value();
    const double expected_res = 5.0;

    expect(res == expected_res);
  };

  "undefined global constant"_test = []()
  {
    MathWorld world;

    auto parsing = parse("cos(1) + my_constant1");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_tree(parsing.value());

    expect(bool(expect_node)) << expect_node;

    const auto res = evaluate(expect_node.value(), world);

    expect(not bool(res)) << res;
  };

  "input var evaluation shadowing a global constant"_test = []()
  {
    MathWorld world;
    world.add<GlobalConstant>("x", 2.0);
    auto parsing = parse("cos(x) + x");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_tree(parsing.value());

    expect(bool(expect_node)) << expect_node;

    const double res = evaluate(expect_node.value(), {{"x", 1.0}}, world).value();

    const double expected_res = std::cos(1.0) + 1.0;
    expect(res == expected_res);
  };

  "wrong object type: function as variable"_test = []
  {
    MathWorld world;
    auto expr = Expression("2 + cos");

    auto eval = expr.evaluate(world);

    expect(not bool(eval));
    expect(eval.error().error_type == eval::Error::WRONG_OBJECT_TYPE);
    expect(eval.error().token.substr_info == SubstrInfo{.begin = 4, .size = 3});
  };

  "wrong object type: variable as function"_test = []
  {
    MathWorld world;
    world.add("g", GlobalConstant(3));
    auto expr = Expression("7 + g(3)");

    auto eval = expr.evaluate(world);

    expect(not bool(eval));
    expect(eval.error().error_type == eval::Error::WRONG_OBJECT_TYPE);
    expect(eval.error().token.substr_info == SubstrInfo{.begin = 4, .size = 1});
  };
}
