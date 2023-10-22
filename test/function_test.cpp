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
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    Function<type, 2>& f = world.template add<Function<type, 2>>("f", Vars<2>{"omega", "t"}, "cos(omega * t) + omega * t").value();

    const double omega = 2;
    const double t = 3;

    // note: the order of the arguments is important
    const double res = f({omega, t}).value();
    const double expected_res = std::cos(omega * t) + omega * t;

    expect(res == expected_res);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "function evaluation shadowing a global constant"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    world.template add<GlobalConstant>("x", 2.0);
    Function<type, 1>& f = world.template add<Function<type, 1>>("f", Vars<1>{"x"}, "cos(x) + x").value();

    const double res = f({1.0}).value();
    const double expected_res = std::cos(1.0) + 1.0;

    expect(res == expected_res);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "function calling another function"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    Function<type, 1>& f1 = world.template add<Function<type, 1>>("f1").value();
    Function<type, 1>& f2 = world.template add<Function<type, 1>>("f2").value();

    f2.set(Vars<1>{"x"}, "cos(x) + 2*x^2");
    f1.set(Vars<1>{"x"}, "cos(x) + x + f2(2*x)");

    expect(bool(f1));
    expect(bool(f2));

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
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;

    GlobalConstant& cst = world.template add<GlobalConstant>("my_constant", 3.0).value();
    Function<type, 1>& f = world.template add<Function<type, 1>>("f", Vars<1>{"x"}, "x + my_constant + cos(math::pi)").value();

    double cpp_cst = 3.0;
    auto cpp_f_1 = [&](double x)
    {
      return x + cpp_cst + std::cos(std::numbers::pi);
    };

    expect(f({1}).value() == cpp_f_1(1.0));

    cst.value = 5.0;
    cpp_cst = 5.0;

    expect(f({1}).value() == cpp_f_1(1.0));

    Function<type, 1>& g = world.template add<Function<type, 1>>("g").value();
    g.set(Vars<1>{"z"}, "2*z + my_constant");

    // override expression and variable name of f
    f.set(Vars<1>{"y"}, "y + my_constant + g(y)");

    auto cpp_g = [&](double z)
    {
      return 2*z + cpp_cst;
    };

    auto cpp_f_2 = [&](double y)
    {
      return y + cpp_cst + cpp_g(y);
    };

    expect(f({3}).value() == cpp_f_2(3));

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "nested multi-variable functions"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;

    world.template add<Function<type, 2>>("h", Vars<2>{"c", "d"}, "c*d + c-d");
    world.template add<Function<type, 2>>("g", Vars<2>{"a", "b"}, "h(a, a*b) + 3*a - b");
    Function<type, 2>& f = world.template add<Function<type, 2>>("f", Vars<2>{"x", "y"}, "h(x, g(x, y)) + g(y, h(y, x))").value();

    auto cpp_h = [](double c, double d) {
      return c*d + c-d;
    };

    auto cpp_g = [&](double a, double b) {
      return cpp_h(a, a*b) + 3*a - b;    };

    auto cpp_f = [&](double x, double y) {
      return cpp_h(x, cpp_g(x, y)) + cpp_g(y, cpp_h(y, x));
    };

    const double x = 5, y = 3;

    expect(f({x, y}).value() == cpp_f(x, y));

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "function with dot in name"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;

    // add a function named "f", note that the constant "my_constant" is only defined after
    Function<type, 1>& fx = world.template add<Function<type, 1>>("f.x", Vars<1>{"x"}, "1 + x").value();
    GlobalVariable<type>& fy = world.template add<GlobalVariable<type>>("f.y", "2.0 + f.x(1)").value();

    expect(fx({1}) == 2.0);
    expect(fy() == 4.0);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "calling function without arguments"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;

    // add a function named "f", note that the constant "my_constant" is only defined after
    world.template add<Function<type, 2>>("f", Vars<2>{"x", "y"}, "1 + x + y").value();
    GlobalVariable<type>& expr = world.template add<GlobalVariable<type>>("val", "1 + f(1)").value();

    expect(bool(expr.error()));
    expect(expr.error().value() == Error::mismatched_fun_args(parsing::tokens::Text("f", 4)))
      << expr.error().value();

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "parametric function benchmark"_test = []<class StructType>()
  {
    {
      constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;
      constexpr std::string_view data_type_str_v = std::is_same_v<StructType, AST_TEST> ? "AST" : "RPN";

      MathWorld<type> world;
      GlobalConstant& t = world.template add<GlobalConstant>("t", 1).value();
      Function<type, 1>& f = world.template add<Function<type, 1>>("f", Vars<1>{"x"}, "3*cos(t*x) + 2*sin(x/t) + 4").value();

      double x = 0;
      auto begin = high_resolution_clock::now();
      double res = 0;
      size_t iterations = 0;
      while (high_resolution_clock::now() - begin < 1s)
      {
        res += f({x}).value();
        iterations++;
        x++;
        t.value++;
      }
      auto end = high_resolution_clock::now();
      std::cout << "Avg zc::Function<" << data_type_str_v << "> eval time: "
                << duration_cast<nanoseconds>((end - begin) / iterations).count() << "ns"
                << std::endl;
      std::cout << "dummy val: " << res << std::endl;
    }
    {
      double cpp_t = 1;
      auto cpp_f = [&](double x) {
        return 3*cos(cpp_t*x) + 2*sin(x/cpp_t) + 4;
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

  "direct dependencies"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;

    // add a function named "f", note that the constant "my_constant" is only defined after
    world.template add<Function<type, 2>>("f", Vars<2>{"x", "y"}, "1 + x + y").value();
    Sequence<type>& seq = world.template add<Sequence<type>>("u", "n", "1 + f(1, 1) + f(2, 2) + u(n-1) + 3*u(n-1) + cos(n)", Vals{0}).value();

    constexpr auto t = deps::ObjectType::FUNCTION;
    expect(seq.direct_dependencies() == std::unordered_map{std::pair{std::string("u"), t}, {"f", t}, {"cos", t}}); // "u" and "f"

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "dependencies"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;

    // add a function named "f", note that the constant "my_constant" is only defined after
    world.template add<Function<type, 2>>("f", Vars<2>{"x", "y"}, "1 + x + y + cos(x)").value();
    world.template add<Function<type, 1>>("g", Vars<1>{"x"}, "1 + x + sin(x)*f(x, x)").value();
    world.template add<Function<type, 1>>("h", Vars<1>{"x"}, "1 + x + 2*g(x)").value();
    Sequence<type>& seq = world.template add<Sequence<type>>("u", "n", "1 + h(n) + u(n-1) + 3*u(n-1)", Vals{0}).value();

    auto t = deps::ObjectType::FUNCTION;

    expect(seq.dependencies() == std::unordered_map{std::pair{std::string("u"), t}, {"f", t}, {"h", t}, {"g", t}, {"sin", t}, {"cos", t}});

  } | std::tuple<AST_TEST, RPN_TEST>{};

}
