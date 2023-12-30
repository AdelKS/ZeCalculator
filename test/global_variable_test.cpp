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

  "dependent expression"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    GlobalConstant<type>& r = world.template add<GlobalConstant<type>>("r").value();
    world.template add<Function<type, 1>>("g", Vars<1>{"x"}, "sin(3 * math::pi * x) + r");
    world.template add<GlobalVariable<type>>("k", "3*g(3)");
    Function<type, 2>& f = world.template add<Function<type, 2>>("f", Vars<2>{"x", "y"}, "cos(math::pi * x) * y + k*g(x) + r").value();

    double cpp_r = 3;
    r = cpp_r;

    auto cpp_g = [&](double x){
      return sin(3 * std::numbers::pi * x) + cpp_r;
    };

    auto cpp_k = [&]{
      return 3 * cpp_g(3);
    };

    auto cpp_f = [&](double x, double y) {
      return cos(std::numbers::pi * x) * y + cpp_k() * cpp_g(x) + cpp_r;
    };

    double x = 7, y = 8;

    auto eval = f({x, y});
    if (not eval)
    {
      auto error = eval.error();
      std::cout << error << std::endl;
    }

    expect(eval.value() == cpp_f(x, y));

    cpp_r = 10;
    r = cpp_r;

    expect(f({x, y}).value() == cpp_f(x, y));

  } | std::tuple<AST_TEST, RPN_TEST>{};
}
