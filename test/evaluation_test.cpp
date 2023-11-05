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
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;

    expect(world.evaluate("cos(2)").value() == std::cos(2.0));

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "simple expression evaluation"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;

    expect(world.evaluate("2+2*2").value() == 6._d);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "complex expression evaluation"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST
                                                                        : parsing::Type::RPN;

    MathWorld<type> world;

    expect(world.evaluate("2/3+2*2*exp(2)^2.5").value() == 2./3.+2.*2.*std::pow(std::exp(2.), 2.5));

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "global constant expression evaluation"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;

    expect(world.evaluate("2*math::π + math::pi/2").value() == 2.5 * std::numbers::pi);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "global constant registering and evaluation"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    world.template add<GlobalConstant<type>>("my_constant1", 2.0);
    world.template add<GlobalConstant<type>>("my_constant2", 3.0);

    expect(world.evaluate("my_constant1 + my_constant2").value() == 5.0_d);
  } | std::tuple<AST_TEST, RPN_TEST>{};

  "undefined global constant"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;

    auto error = world.evaluate("cos(1) + my_constant1").error();

    expect(error == zc::Error::undefined_variable(parsing::tokens::Variable("my_constant1", 9, 12))) << error;

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "input var evaluation shadowing a global constant"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    world.template add<GlobalConstant<type>>("x", 2.0);

    Function<type, 1>& fun = world.template add<Function<type, 1>>("f", Vars<1>{"x"}, "cos(x) + x").value();

    const double res = fun({1.0}).value();

    const double expected_res = std::cos(1.0) + 1.0;

    expect(res == expected_res);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "wrong object type: function as variable"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;

    auto error = world.evaluate("2 + cos").error();

    expect(error.type == Error::WRONG_OBJECT_TYPE);
    expect(error.token.substr_info == SubstrInfo{.begin = 4, .size = 3});

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "wrong object type: variable as function"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    world.template add<GlobalConstant<type>>("g", 3);

    auto error = world.evaluate("7 + g(3)").error();
    expect(error.type == Error::WRONG_OBJECT_TYPE);
    expect(error.token.substr_info == SubstrInfo{.begin = 4, .size = 1});

  } | std::tuple<AST_TEST, RPN_TEST>{};
}
