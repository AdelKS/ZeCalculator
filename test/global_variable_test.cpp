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
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::AST : parsing::RPN;

    MathWorld<type> world;
    auto f = world.add("f", Function<type>({"x", "y"}, "cos(math::pi * x) * y + k*g(x) + r")).value();
    auto r = world.add("r", GlobalConstant()).value();
    world.add("k", GlobalVariable<type>("3*g(3)"));
    world.add("g", Function<type>({"x"}, "sin(3 * math::pi * x) + r"));

    double cpp_r = 3;
    *r = cpp_r;

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

    expect(f({x, y}).value() == cpp_f(x, y));

    cpp_r = 10;
    *r = cpp_r;

    expect(f({x, y}).value() == cpp_f(x, y));
  } | std::tuple<AST_TEST, RPN_TEST>{};
}
