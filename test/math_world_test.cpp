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

#include "zecalculator/mathworld.h"
#include <zecalculator/utils/parser.h>

// testing specific headers
#include <boost/ut.hpp>
#include <zecalculator/test-utils/print-utils.h>

using namespace zc;

int main()
{
  using namespace boost::ut;

  "simple test"_test = []()
  {
    MathWorld world;
    expect((*world.get<CppUnaryFunction>("sqrt").value())(4) == 2);
  };

  "Add constant then set value"_test = []()
  {
    MathWorld world;
    auto c1 = world.add<GlobalConstant>("my_constant1").value();
    *c1 = 2.0;
    expect(c1->value == 2.0);
  };

  "Add same constant twice"_test = []()
  {
    MathWorld world;
    world.add<GlobalConstant>("my_constant1", 2.0);
    expect(not world.add<GlobalConstant>("my_constant1", 3.0).has_value());
  };

  return 0;
}
