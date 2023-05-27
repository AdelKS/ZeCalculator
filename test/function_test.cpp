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

  "multi-parameter function evaluation"_test = []()
  {
    MathWorld world;
    auto f = Function({"omega", "t"}, "cos(omega * t) + omega * t");

    const double omega = 2;
    const double t = 3;

    // note: the order of the arguments is important
    const double res = f({omega, t}, world).value();
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
    auto f1 = world.add<Function>("f1").value();
    auto f2 = world.add<Function>("f2").value();

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

  "function overwrites"_test = []{
    MathWorld world;

    // add a function named "f", note that the constant "my_constant" is only defined after
    auto f = world.add<Function>("f", Function({"x"}, "x + my_constant + cos(math::pi)")).value();

    // add a global constant called "my_constant" with an initial value of 3.0
    auto cst = world.add<GlobalConstant>("my_constant", 3.0).value();

    double cpp_cst = 3.0;
    auto cpp_f_1 = [&](double x)
    {
      return x + cpp_cst + std::cos(std::numbers::pi);
    };

    expect(f({1}).value() == cpp_f_1(1.0));

    *cst = 5.0;
    cpp_cst = 5.0;

    expect(f({1}).value() == cpp_f_1(1.0));

    // override expression and variable name of f
    *f = Function({"y", "z"}, "y + z + my_constant + g(y)");

    auto cpp_g = [&](double z)
    {
      return 2*z + cpp_cst;
    };

    auto cpp_f_2 = [&](double y, double z)
    {
      return y + z + cpp_cst + cpp_g(y);
    };

    auto g = world.add<Function>("g").value();
    *g = Function({"z"}, "2*z + my_constant");

    expect(f({3, 4}).value() == cpp_f_2(3, 4));
  };

  "nested multi-variable functions"_test = []{
    MathWorld world;

    // add a function named "f", note that the constant "my_constant" is only defined after
    auto f = world.add<Function>("f", Function({"x", "y"}, "h(x, g(x, y)) + g(y, h(y, x))")).value();
    world.add<Function>("g", Function({"a", "b"}, "h(a, a*b) + 3*a - b"));
    world.add<Function>("h", Function({"c", "d"}, "c*d + c-d"));

    auto cpp_h = [](double c, double d) {
      return c*d + c-d;
    };

    auto cpp_g = [&](double a, double b) {
      return cpp_h(a, a*b) + 3*a - b;
    };

    auto cpp_f = [&](double x, double y) {
      return cpp_h(x, cpp_g(x, y)) + cpp_g(y, cpp_h(y, x));
    };

    const double x = 5, y = 3;

    expect(f({x, y}).value() == cpp_f(x, y));
  };

}
