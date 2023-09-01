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

using namespace zc;
using namespace std::chrono;

int main()
{
  using namespace boost::ut;

  "multi-parameter function evaluation"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::AST : parsing::RPN;

    MathWorld<type> world;
    auto f = Function<type>({"omega", "t"}, "cos(omega * t) + omega * t");

    const double omega = 2;
    const double t = 3;

    // note: the order of the arguments is important
    const double res = f({omega, t}, world).value();
    const double expected_res = std::cos(omega * t) + omega * t;

    expect(res == expected_res);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "function evaluation shadowing a global constant"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::AST : parsing::RPN;

    MathWorld<type> world;
    world.add("x", GlobalConstant(2.0));
    auto f = Function<type>({"x"}, "cos(x) + x");

    const double res = f({1.0}, world).value();
    const double expected_res = std::cos(1.0) + 1.0;

    expect(res == expected_res);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "function calling another function"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::AST : parsing::RPN;

    MathWorld<type> world;
    auto f1 = world.add("f1", Function<type>()).value();
    auto f2 = world.add("f2", Function<type>()).value();

    *f1 = Function<type>({"x"}, "cos(x) + x + f2(2*x)");
    *f2 = Function<type>({"x"}, "cos(x) + 2*x^2");

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

    using namespace boost::ut::literals;

    if (res2_status)
      expect(expected_res2.value() - cpp_f2(2*x) == 0.0_d);

    const auto expected_res1 = f1({x});

    expect(bool(expected_res1));

    if (bool(expected_res1))
      expect(std::fabs(expected_res1.value() - cpp_f1(x)) < 1e-11) << expected_res1.value() << " = " << cpp_f1(x);
  } | std::tuple<AST_TEST, RPN_TEST>{};

  "function overwrites"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::AST : parsing::RPN;

    MathWorld<type> world;

    // add a function named "f", note that the constant "my_constant" is only defined after
    auto f = world.add("f", Function<type>({"x"}, "x + my_constant + cos(math::pi)")).value();

    // add a global constant called "my_constant" with an initial value of 3.0
    auto cst = world.add("my_constant", GlobalConstant(3.0)).value();

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
    *f = Function<type>({"y", "z"}, "y + z + my_constant + g(y)");

    auto cpp_g = [&](double z)
    {
      return 2*z + cpp_cst;
    };

    auto cpp_f_2 = [&](double y, double z)
    {
      return y + z + cpp_cst + cpp_g(y);
    };

    auto g = world.add("g", Function<type>()).value();
    *g = Function<type>({"z"}, "2*z + my_constant");

    expect(f({3, 4}).value() == cpp_f_2(3, 4));

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "nested multi-variable functions"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::AST : parsing::RPN;

    MathWorld<type> world;

    // add a function named "f", note that the constant "my_constant" is only defined after
    auto f = world.add("f", Function<type>({"x", "y"}, "h(x, g(x, y)) + g(y, h(y, x))")).value();
    world.add("g", Function<type>({"a", "b"}, "h(a, a*b) + 3*a - b"));
    world.add("h", Function<type>({"c", "d"}, "c*d + c-d"));

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
  } | std::tuple<AST_TEST, RPN_TEST>{};

  "function with dot in name"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::AST : parsing::RPN;

    MathWorld<type> world;

    // add a function named "f", note that the constant "my_constant" is only defined after
    auto fx = world.add("f.x", Function<type>({"x"}, "1 + x")).value();
    auto fy = world.add("f.y", Expression<type>("2.0 + f.x(1)")).value();

    expect(fx({1}) == 2.0);
    expect(fy() == 4.0);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "calling function without arguments"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::AST : parsing::RPN;

    MathWorld<type> world;

    // add a function named "f", note that the constant "my_constant" is only defined after
    world.add("f", Function<type>({"x", "y"}, "1 + x + y")).value();
    auto expr = world.add("val", Expression<type>("1 + f(1)")).value();

    [[maybe_unused]] const auto& parsing = expr->get_parsing().value();

    if constexpr (type == parsing::AST)
    {
      expect(expr()
             == tl::unexpected(eval::Error::mismatched_fun_args(
               ast::node::Function(parsing::tokens::Text("f", 4, 1),
                                   {ast::node::Number(1.0, parsing::tokens::Text("1", 6, 1))}))))
        << expr();
    }
    else
    {
      // error message in postfix notation isn't accurate, because the expression is translated to
      // "1 + f(1)" -> "1, 1, f, +" and 'f' will pick the first two values, then '+' ends up with just one
      // value
      expect(expr()
             == tl::unexpected(eval::Error::mismatched_fun_args(parsing::tokens::Operator('+', 2))))
        << expr();
    }


  } | std::tuple<AST_TEST, RPN_TEST>{};

  "parametric function benchmark"_test = []<class StructType>()
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
