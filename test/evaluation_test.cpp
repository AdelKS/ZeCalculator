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

#include "zecalculator/parsing/data_structures/token.h"
#include <zecalculator/zecalculator.h>

// testing specific headers
#include <boost/ut.hpp>
#include <zecalculator/test-utils/print-utils.h>
#include <zecalculator/test-utils/structs.h>
#include <zecalculator/test-utils/utils.h>

using namespace zc;

int main()
{
  using namespace boost::ut;

  "simple function evaluation"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;

    expect(world.evaluate("cos(2)").value() == std::cos(2.0));

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "simple expression evaluation"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;

    expect(world.evaluate("2+2*2").value() == 6._d);

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "operator same priority"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;

    [[maybe_unused]] auto res = world.evaluate("2+2-2-2");

    expect(world.evaluate("2+2-2+2").value() == 4._d);
    expect(world.evaluate("2+2-2-2").value() == 0._d);
    expect(world.evaluate("2-2+2+2").value() == 4._d);
    expect(world.evaluate("2-2+2-2").value() == 0._d);

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "complex expression evaluation"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST
                                                                        : parsing::Type::RPN;

    MathWorld<type> world;

    expect(world.evaluate("2/3+2*2*exp(2)^2.5").value() == 2./3.+2.*2.*std::pow(std::exp(2.), 2.5));

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "global constant expression evaluation"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;

    expect(world.evaluate("2*math::π + math::pi/2").value() == 2.5 * std::numbers::pi);

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "global constant registering and evaluation"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    world.add("my_constant1 = 2.0");
    world.add("my_constant2 = 3.0");

    expect(world.evaluate("my_constant1 + my_constant2").value() == 5.0_d);
  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "undefined global constant"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;

    std::string expression = "cos(1) + my_constant1";

    auto error = world.evaluate(expression).error();

    expect(error
           == zc::Error::undefined_variable(parsing::Token(parsing::tokens::VARIABLE,
                                                           parsing::tokens::Text{"my_constant1", 9}),
                                            expression))
      << error;

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "input var evaluation shadowing a global constant"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    world.add("x  =   2.0");

    Function<type>& fun = world.add("f(x) = cos(x) + x").template value_as<Function<type>>();

    const double res = fun(1.0).value();

    const double expected_res = std::cos(1.0) + 1.0;

    expect(res == expected_res);

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "wrong object type: function as variable"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;

    auto error = world.evaluate("2 + cos").error();

    expect(error.type == Error::WRONG_OBJECT_TYPE);
    expect(error.token == parsing::tokens::Text{"cos", 4});

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "wrong object type: variable as function"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    world.add("g = 3");

    auto error = world.evaluate("7 + g(3)").error();
    expect(error.type == Error::WRONG_OBJECT_TYPE);
    expect(error.token == parsing::tokens::Text{"g", 4});

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "random separators"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;

    expect(not bool(world.evaluate("7 + (3, 5)")));
    expect(not bool(world.evaluate("7 , (3, 5)")));
    expect(not bool(world.evaluate("cos(,3)")));
    expect(not bool(world.evaluate("sin(3;3)")));

    auto& obj = world.add("f(x) = 3, 5");
    expect(not obj.has_value()) << fatal;
    expect(obj.error().token == parsing::tokens::Text{",", 8});
    expect(obj.error().type == Error::UNEXPECTED);

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "AST/FAST/RPN creation speed"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;
    MathWorld<type> world;

    constexpr std::string_view static_expr = "2+ 3 -  cos(x) - 2 + 3 * 2.5343E+12-34234+2-4 * 34 / 634534           + 45.4E+2";
    constexpr size_t static_expr_size = static_expr.size();
    constexpr auto duration = nanoseconds(1s);

    constexpr size_t max_random_padding = 10;
    size_t dummy = 0;
    std::string expr(static_expr);
    expr.reserve(static_expr.size() + max_random_padding);

    size_t i = 0;
    size_t iterations = loop_call_for(duration, [&]{
      // resize with variable number of extra spaces
      // just to fool the compiler so it thinks each call to this function is unique
      i = (i + 1) % max_random_padding;
      expr.resize(static_expr_size + i, ' ');

      if constexpr (std::is_same_v<StructType, AST_TEST>)
      {
        auto exp_ast = parsing::tokenize(expr).and_then(parsing::make_ast{expr});
        dummy += exp_ast->dyn_data.index();
      }
      else if constexpr (std::is_same_v<StructType, FAST_TEST>)
      {
        auto exp_ast = parsing::tokenize(expr)
                         .and_then(parsing::make_ast{expr})
                         .and_then(parsing::make_fast<type>{expr, world});
        dummy += exp_ast->node.index();
      }
      else
      {
        static_assert(std::is_same_v<StructType, RPN_TEST>);
        auto exp_ast = parsing::tokenize(expr)
                         .and_then(parsing::make_ast{expr})
                         .and_then(parsing::make_fast<type>{expr, world})
                         .transform(parsing::make_RPN);
        dummy += exp_ast->size();
      }
    });

    constexpr std::string_view type_str_v = std::is_same_v<StructType, AST_TEST> ? "AST" :
                                            std::is_same_v<StructType, FAST_TEST> ? "FAST" : "RPN";

    // the absolute value doesn't mean anything really, but we can compare between performance improvements
    std::cout << type_str_v << " creation time: "
              << duration_cast<nanoseconds>(duration/iterations).count() << "ns"
              << std::endl;
    std::cout << "dummy: " << dummy << std::endl;

  } | std::tuple<AST_TEST, FAST_TEST, RPN_TEST>{};
}
