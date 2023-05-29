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

  "fibonacci sequence"_test = []()
  {
    MathWorld world;
    auto fib = world.add("fib", Sequence("n", "fib(n-1) + fib(n-2)", {0, 1})).value();

    expect(fib(0).value() == 0.0);
    expect(fib(1).value() == 1.0);
    expect(fib(2).value() == 1.0);
    expect(fib(3).value() == 2.0);
    expect(fib(4).value() == 3.0);
    expect(fib(10).value() == 55.0);
  };

  "recursion depth overflow"_test = []()
  {
    MathWorld world;
    auto bad = world.add("bad", Sequence("n", "bad(n+10) + bad(n+20)", {})).value();

    expect(bad(0).error() == EvaluationError::recursion_depth_overflow());
  };
}
