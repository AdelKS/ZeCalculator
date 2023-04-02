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
    world.add_global_constant("x", 2.0);
    auto f = Function({"x"}, "cos(x) + x");

    const double res = f({1.0}, world).value();
    const double expected_res = std::cos(1.0) + 1.0;

    expect(res == expected_res);
  };
}
