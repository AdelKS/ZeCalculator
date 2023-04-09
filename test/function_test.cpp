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

#include <zecalculator/function.h>
#include <zecalculator/mathworld.h>

// testing specific headers
#include <boost/ut.hpp>
#include <zecalculator/test-utils/print-utils.h>

using namespace zc;

int main()
{
  using namespace boost::ut;

  "multi-parameter function evaluation"_test = []()
  {
    auto f = Function({"omega", "t"}, "cos(omega * t) + omega * t");

    const double omega = 2;
    const double t = 3;

    // note: the order of the arguments is important
    const double res = f({omega, t}, global_world).value();
    const double expected_res = std::cos(omega * t) + omega * t;

    expect(res == expected_res);
  };

  "function evaluation shadowing a global constant"_test = []()
  {
    MathWorld world;
    world.add<GlobalConstant>("x", 2.0);
    auto f = Function({"x"}, "cos(x) + x");

    const double res = f({1.0}, world).value();
    const double expected_res = std::cos(1.0) + 1.0;

    expect(res == expected_res);
  };

  "function calling another function"_test = []()
  {
    MathWorld world;
    auto f1 = world.add<Function>("f1");
    auto f2 = world.add<Function>("f2");

    *f1 = Function({"x"}, "cos(x) + x + f2(2*x)");
    *f2 = Function({"x"}, "cos(x) + 2*x^2");

    expect(bool(*f1));
    expect(bool(*f2));

    auto cpp_f2 = [](double x)
    {
      return std::cos(x) + 2*x*x;
    };
    auto cpp_f1 = [cpp_f2](double x)
    {
      return std::cos(x) + x + cpp_f2(2*x);
    };

    double x = 6.4;

    const auto expected_res2 = f2({2*x});

    const bool res2_status = bool(expected_res2);

    expect(res2_status);

    if (res2_status)
      expect(expected_res2.value() == cpp_f2(2*x));

    const auto expected_res1 = f1({x});

    expect(bool(expected_res1));

    if (bool(expected_res1))
      expect(std::fabs(expected_res1.value() - cpp_f1(x)) < 1e-11) << expected_res1.value() << " = " << cpp_f1(x);
  };

}
