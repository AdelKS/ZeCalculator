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
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    const double t = 3;

    MathWorld<type> world;

    world.add("f( x, y)  = x + y");
    world.add("t = " + std::to_string(t));

    GlobalVariable<type>& expr
      = world.add("test_var = cos(math::pi * t) + 2 + f(3, 4)").template value_as<GlobalVariable<type>>();

    auto cpp_f = [](double x, double y) {
      return x + y;
    };

    auto cpp_expr = [&]{
      return std::cos(std::numbers::pi * t) + 2 + cpp_f(3, 4);
    };

    auto expr_eval = expr();
    if (not expr_eval)
    {
      auto error = expr_eval.error();
      std::cout << error << std::endl;
    }

    expect(expr().value() == cpp_expr());

  } | std::tuple<FAST_TEST, RPN_TEST>{};

}
