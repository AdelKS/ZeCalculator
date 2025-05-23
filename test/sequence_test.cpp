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

  "fibonacci sequence"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    auto& fib = world.new_object() = "fib(n) = 0 ; 1 ; fib(n-1) + fib(n-2)";

    expect(bool(fib)) << [&] { return fib.error(); } << fatal;

    expect(bool(fib({0}))) << [&]{ return fib({0}).error(); } << fatal;

    expect(fib({0}).value() == 0.0_d);
    expect(fib({1}).value() == 1.0_d);
    expect(fib({2}).value() == 1.0_d);
    expect(fib({3}).value() == 2.0_d);
    expect(fib({4}).value() == 3.0_d);
    expect(fib({10}).value() == 55.0_d);

    auto* fib_obj = world.get("fib");
    expect(fib_obj && (*fib_obj)({10}).value() == 55.0_d);

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "fibonacci sequence with cache"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    eval::Cache cache;

    MathWorld<type> world;
    auto& fib = world.new_object() = "fib(n) = 0 ; 1 ; fib(n-1) + fib(n-2)";

    expect(bool(fib)) << [&] { return fib.error(); } << fatal;

    expect(bool(fib({0}, &cache))) << [&]{ return fib({0}).error(); } << fatal;

    expect(fib({0}, &cache).value() == 0.0_d);
    expect(fib({1}, &cache).value() == 1.0_d);
    expect(fib({2}, &cache).value() == 1.0_d);
    expect(fib({3}, &cache).value() == 2.0_d);
    expect(fib({4}, &cache).value() == 3.0_d);
    expect(fib({10}, &cache).value() == 55.0_d);
    expect(fib({20}, &cache).value() == 6765.0_d);
    expect(fib({30}, &cache).value() == 832040.0_d);

    auto* fib_obj = world.get("fib");
    expect(fib_obj && (*fib_obj)({10}, &cache).value() == 55.0_d);

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "recursion depth overflow"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    auto& bad = world.new_object() = "bad(n) = bad(n+10) + bad(n+20)";

    expect(bad({0}).error() == Error::recursion_depth_overflow());

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "invalid function depending on invalid sequence"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    auto& f = world.new_object() = "f(x) = cos(x) + u(n)";
    auto& u = world.new_object() = "u(n) = 1 ; 1 ; u";

    expect(f.error().value().type == Error::UNDEFINED_VARIABLE) << f.error().value().type;
    expect(f.error().value().token == parsing::Token::Variable("n", 18)) << f.error().value().token;

    expect(u.error().value().type == Error::WRONG_OBJECT_TYPE) << u.error().value().type;
    expect(u.error().value().token == parsing::Token::Variable("u", 15)) << u.error().value().token;

  } | std::tuple<FAST_TEST, RPN_TEST>{};
}
