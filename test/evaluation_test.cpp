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

int main()
{
  using namespace boost::ut;

  "simple function evaluation"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::AST : parsing::RPN;

    MathWorld<type> world;

    auto expr = Expression<type>("cos(2)");

    expect(expr.evaluate(world).value() == std::cos(2.0));

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "simple expression evaluation"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::AST : parsing::RPN;

    MathWorld<type> world;

    auto expr = Expression<type>("2+2*2");

    expect(expr.evaluate(world).value() == 6);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "complex expression evaluation"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::AST : parsing::RPN;

    MathWorld<type> world;

    auto expr = Expression<type>("2/3+2*2*exp(2)^2.5");

    const double res = expr.evaluate(world).value();
    const double expected_res = 2./3.+2.*2.*std::pow(std::exp(2.), 2.5);

    expect(res == expected_res);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "global constant expression evaluation"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::AST : parsing::RPN;

    MathWorld<type> world;

    auto expr = Expression<type>("2*math::π + math::pi/2");

    const double res = expr.evaluate(world).value();
    const double expected_res = 2.5 * std::numbers::pi;

    expect(res == expected_res);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "global constant registering and evaluation"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::AST : parsing::RPN;

    MathWorld<type> world;
    world.add("my_constant1", GlobalConstant(2.0));
    world.add("my_constant2", GlobalConstant(3.0));

    auto expr = Expression<type>("my_constant1 + my_constant2");

    const double res = expr.evaluate(world).value();
    const double expected_res = 5.0;

    expect(res == expected_res);
  } | std::tuple<AST_TEST, RPN_TEST>{};

  "undefined global constant"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::AST : parsing::RPN;

    MathWorld<type> world;

    auto expr = Expression<type>("cos(1) + my_constant1");

    const auto res = expr.evaluate(world);

    expect(not bool(res)) << res;

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "input var evaluation shadowing a global constant"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::AST : parsing::RPN;

    MathWorld<type> world;
    world.add("x", GlobalConstant(2.0));

    auto fun = Function<type>({"x"}, "cos(x) + x");

    const double res = fun.evaluate({1.0}, world).value();

    const double expected_res = std::cos(1.0) + 1.0;

    expect(res == expected_res);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "wrong object type: function as variable"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::AST : parsing::RPN;

    MathWorld<type> world;
    auto expr = Expression<type>("2 + cos");

    auto eval = expr.evaluate(world);

    expect(not bool(eval));
    expect(eval.error().error_type == Error::WRONG_OBJECT_TYPE);
    expect(eval.error().token.substr_info == SubstrInfo{.begin = 4, .size = 3});

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "wrong object type: variable as function"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::AST : parsing::RPN;

    MathWorld<type> world;
    world.add("g", GlobalConstant(3));
    auto expr = Expression<type>("7 + g(3)");

    auto eval = expr.evaluate(world);

    expect(not bool(eval));
    expect(eval.error().error_type == Error::WRONG_OBJECT_TYPE);
    expect(eval.error().token.substr_info == SubstrInfo{.begin = 4, .size = 1});

  } | std::tuple<AST_TEST, RPN_TEST>{};
}
