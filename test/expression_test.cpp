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
#include <chrono>
#include <zecalculator/test-utils/print-utils.h>
#include <zecalculator/test-utils/structs.h>

using namespace std::chrono;

using namespace zc;

int main()
{
  using namespace boost::ut;

  "dependent expression"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::AST : parsing::RPN;

    MathWorld<type> world;
    auto expr = Expression<type>("cos(math::pi * t) + 2 + f(3, 4)");

    const double t = 3;

    world.add("f", Function<type>({"x", "y"}, "x + y"));
    world.add("t", GlobalConstant(t));

    auto cpp_f = [](double x, double y) {
      return x + y;
    };

    auto cpp_expr = [&]{
      return std::cos(std::numbers::pi * t) + 2 + cpp_f(3, 4);
    };

    expect(expr(world).value() == cpp_expr());

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "expression benchmark"_test = []<class StructType>()
  {
    {
      constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::AST : parsing::RPN;
      constexpr std::string_view data_type_str_v = std::is_same_v<StructType, AST_TEST> ? "AST" : "RPN";

      MathWorld<type> world;
      auto f = world.add("f", Function<type> ({"x"}, "cos(x) + t + 3")).value();
      auto t = world.add("t", GlobalConstant(0)).value();

      double x = 0;
      auto begin = high_resolution_clock::now();
      double res = 0;
      size_t iterations = 0;
      while (high_resolution_clock::now() - begin < 1s)
      {
        res += f({x}).value();
        iterations++;
        x++;
        t->value++;
      }
      auto end = high_resolution_clock::now();
      std::cout << "Avg zc::Function<" << data_type_str_v << "> eval time: "
                << duration_cast<nanoseconds>((end - begin) / iterations).count() << "ns"
                << std::endl;
      std::cout << "dummy val: " << res << std::endl;
    }
    {
      double cpp_t = 0;
      auto cpp_f = [&](double x) {
        return cos(x) + cpp_t + 3;
      };

      double x = 0;
      auto begin = high_resolution_clock::now();
      double res = 0;
      size_t iterations = 0;
      while (high_resolution_clock::now() - begin < 1s)
      {
        res += cpp_f(x);
        iterations++;
        x++;
        cpp_t++;
      }
      auto end = high_resolution_clock::now();
      std::cout << "Avg C++ function eval time: " << duration_cast<nanoseconds>((end - begin)/iterations).count() << "ns" << std::endl;
      std::cout << "dummy val: " << res << std::endl;

    }

  } | std::tuple<AST_TEST, RPN_TEST>{};
}
