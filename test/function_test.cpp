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
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    auto& f = world.add("f(omega,t) = cos(omega * t) + omega * t");

    const double omega = 2;
    const double t = 3;

    // note: the order of the arguments is important
    const double res = f(omega, t).value();
    const double expected_res = std::cos(omega * t) + omega * t;

    expect(res == expected_res);

    // dynamic object tests
    auto* obj = world.get("f");
    expect(obj && (*obj)(omega, t).value() == expected_res);

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "function evaluation shadowing a global constant"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    world.add("x = 2.0");
    auto& f = world.add("f(x) = cos(x) + x");

    const double res = f(1.0).value();
    const double expected_res = std::cos(1.0) + 1.0;

    expect(res == expected_res);

    // dynamic object tests
    auto* x_obj = world.get("x");
    expect(x_obj && (*x_obj)().value() == 2.0);

    auto* f_obj = world.get("f");
    expect(f_obj && (*f_obj)(1.0).value() == expected_res);

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "function calling another function"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    auto& f1 = world.add("");
    auto& f2 = world.add("");

    world.redefine(f2, "f2(x) = cos(x) + 2*x^2");
    world.redefine(f1, "f1(x) = cos(x) + x + f2(2*x)");

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

    const auto expected_res2 = f2(2*x);

    const bool res2_status = bool(expected_res2);

    expect(res2_status);

    using namespace boost::ut::literals;

    if (res2_status)
      expect(expected_res2.value() - cpp_f2(2*x) == 0.0_d);

    const auto expected_res1 = f1(x);

    expect(bool(expected_res1));

    if (bool(expected_res1))
      expect(std::fabs(expected_res1.value() - cpp_f1(x)) < 1e-11) << expected_res1.value() << " = " << cpp_f1(x);
  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "function overwrites"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;

    auto& cst = world.add("my_constant = 3.0");
    auto& f = world.add("f(x) = x + my_constant + cos(math::pi)");

    auto* f_obj = world.get("f");
    expect(f_obj);

    double cpp_cst = 3.0;
    auto cpp_f_1 = [&](double x)
    {
      return x + cpp_cst + std::cos(std::numbers::pi);
    };

    expect(f(1).value() == cpp_f_1(1.0));
    expect((*f_obj)(1) == cpp_f_1(1.0));

    cst.template value_as<GlobalConstant<type>>() = 5.0;
    cpp_cst = 5.0;

    expect(f(1).value() == cpp_f_1(1.0));
    expect((*f_obj)(1).value() == cpp_f_1(1.0));

    world.add("g(z) = 2*z + my_constant");

    // override expression and variable name of f
    world.redefine(f, "f(y) = y + my_constant + g(y)");

    auto cpp_g = [&](double z)
    {
      return 2*z + cpp_cst;
    };

    auto cpp_f_2 = [&](double y)
    {
      return y + cpp_cst + cpp_g(y);
    };

    expect(f(3).value() == cpp_f_2(3));
    expect((*f_obj)(3).value() == cpp_f_2(3));

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "nested multi-variable functions"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;

    world.add("h(c,d) = c*d + c-d");
    world.add("g(a,b) = h(a, a*b) + 3*a - b");
    auto& f = world.add("f(x, y) = h(x, g(x, y)) + g(y, h(y, x))");

    auto cpp_h = [](double c, double d) {
      return c*d + c-d;
    };

    auto cpp_g = [&](double a, double b) {
      return cpp_h(a, a*b) + 3*a - b;    };

    auto cpp_f = [&](double x, double y) {
      return cpp_h(x, cpp_g(x, y)) + cpp_g(y, cpp_h(y, x));
    };

    const double x = 5, y = 3;

    expect(f(x, y).value() == cpp_f(x, y));

    auto* f_obj = world.get("f");
    expect(f_obj && (*f_obj)(x, y).value() == cpp_f(x, y));

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "function with dot in name"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;

    // add a function named "f", note that the constant "my_constant" is only defined after
    auto& fx = world.add("f.x(x) = 1 + x");
    auto& fy = world.add("f.y = 2.0 + f.x(1)");

    expect(fx(1).value() == 2.0);
    expect(fy().value() == 4.0);

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "calling function with wrong number of arguments"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;

    // add a function named "f", note that the constant "my_constant" is only defined after
    world.add("f(x, y) = 1 + x + y");

    std::string var_expr = "1 + f(1, 2, 3)";
    tl::expected<double, Error> res = world.evaluate(var_expr);

    expect(not res.has_value());
    expect(res.error() == Error::mismatched_fun_args(parsing::tokens::Text("1, 2, 3", 6), var_expr))
      << res.error();

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "parametric function benchmark"_test = []<class StructType>()
  {
    {
      constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;
      constexpr std::string_view data_type_str_v = std::is_same_v<StructType, FAST_TEST> ? "FAST" : "RPN";

      MathWorld<type> world;
      auto& t = world.add("t = 1").template value_as<GlobalConstant<type>>();
      auto& f = world.add("f(x) =3*cos(t*x) + 2*sin(x/t) + 4").template value_as<Function<type, 1>>();

      double x = 0;
      auto begin = high_resolution_clock::now();
      double res = 0;
      size_t iterations = 0;
      while (high_resolution_clock::now() - begin < 1s)
      {
        res += f({x}).value();
        iterations++;
        x++;
        t.set_fast(t.value()+1);
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

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "sequence direct dependencies"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;

    // add a function named "f", note that the constant "my_constant" is only defined after
    world.add("f(x, y) = 1 + x + y");
    auto& seq = world.template add<Sequence<type>>("u(n) = 1 + f(1, 1) + f(2, 2) + u(n-1) + 3*u(n-1) + cos(n)").template value_as<Sequence<type>>();
    seq.set_first_values(Vals{0});

    constexpr auto t = deps::ObjectType::FUNCTION;
    expect(seq.direct_dependencies() == std::unordered_map{std::pair{std::string("u"), t}, {"f", t}, {"cos", t}}); // "u" and "f"

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "function direct dependencies"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;

    // add a function named "f", note that the constant "my_constant" is only defined after
    world.add("my_constant = 3.0");
    Function<type, 1>& f = world.add("f( x)  = x + my_constant + cos(math::pi)").template value_as<Function<type, 1>>();

    using namespace std::string_literals;

    expect(f.direct_dependencies()
           == std::unordered_map{std::pair(std::string("my_constant"), deps::VARIABLE),
                                 {"cos", deps::FUNCTION},
                                 {"math::pi", deps::VARIABLE}}); // "u" and "f"

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "dependencies"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;

    // add a function named "f", note that the constant "my_constant" is only defined after
    world.add("f(x, y ) = 1 + x + y + cos(x)");
    world.add("g( x) = 1 + x + sin(x)*f(x, x)");
    world.add("h(x) =  1 + x + 2*g(x)");
    Sequence<type>& seq = world.template add<Sequence<type>>("u(n) = 1 + h(n) + u(n-1) + 3*u(n-1)").template value_as<Sequence<type>>();

    auto t = deps::ObjectType::FUNCTION;

    expect(seq.dependencies() == std::unordered_map{std::pair{std::string("u"), t}, {"f", t}, {"h", t}, {"g", t}, {"sin", t}, {"cos", t}});

  } | std::tuple<FAST_TEST, RPN_TEST>{};

}
