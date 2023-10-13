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

    GlobalVariable<type>& expr = world.template add<GlobalVariable<type>>("foo_var", "cos(2)").value();

    expect(expr().value() == std::cos(2.0));

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "simple expression evaluation"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;

    GlobalVariable<type>& expr = world.template add<GlobalVariable<type>>("foo_var", "2+2*2").value();

    expect(expr().value() == 6);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "complex expression evaluation"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST
                                                                        : parsing::Type::RPN;

    MathWorld<type> world;

    GlobalVariable<type>& expr
      = world.template add<GlobalVariable<type>>("foo_var", "2/3+2*2*exp(2)^2.5").value();

    const double res = expr().value();
    const double expected_res = 2./3.+2.*2.*std::pow(std::exp(2.), 2.5);

    expect(res == expected_res);
  } | std::tuple<AST_TEST, RPN_TEST>{};

  "global constant expression evaluation"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;

    GlobalVariable<type>& expr = world.template add<GlobalVariable<type>>("foo_var", "2*math::π + math::pi/2").value();

    const double res = expr().value();
    const double expected_res = 2.5 * std::numbers::pi;

    expect(res == expected_res);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "global constant registering and evaluation"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    world.template add<GlobalConstant>("my_constant1", 2.0);
    world.template add<GlobalConstant>("my_constant2", 3.0);

    GlobalVariable<type>& expr = world.template add<GlobalVariable<type>>("foo_var", "my_constant1 + my_constant2").value();

    const double res = expr().value();
    const double expected_res = 5.0;

    expect(res == expected_res);
  } | std::tuple<AST_TEST, RPN_TEST>{};

  "undefined global constant"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;

    GlobalVariable<type>& expr = world.template add<GlobalVariable<type>>("foo_var", "cos(1) + my_constant1").value();

    expect(bool(expr.error()));

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "input var evaluation shadowing a global constant"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    world.template add<GlobalConstant>("x", 2.0);

    Function<type, 1>& fun = world.template add<Function<type, 1>>("f", Vars<1>{"x"}, "cos(x) + x").value();

    const double res = fun({1.0}).value();

    const double expected_res = std::cos(1.0) + 1.0;

    expect(res == expected_res);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "wrong object type: function as variable"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    GlobalVariable<type>& expr = world.template add<GlobalVariable<type>>("foo_var", "2 + cos").value();

    auto has_error = expr.error();

    expect(bool(has_error));

    auto error = has_error.value();

    expect(error.error_type == Error::WRONG_OBJECT_TYPE);
    expect(error.token.substr_info == SubstrInfo{.begin = 4, .size = 3});

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "wrong object type: variable as function"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    world.template add<GlobalConstant>("g", 3);
    GlobalVariable<type>& expr = world.template add<GlobalVariable<type>>("foo_var", "7 + g(3)").value();

    auto has_error = expr.error();

    expect(bool(has_error));

    auto error = has_error.value();
    expect(error.error_type == Error::WRONG_OBJECT_TYPE);
    expect(error.token.substr_info == SubstrInfo{.begin = 4, .size = 1});

  } | std::tuple<AST_TEST, RPN_TEST>{};
}
