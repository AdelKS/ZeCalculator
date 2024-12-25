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
using namespace zc::parsing;

int main()
{
  using namespace boost::ut;

  "simple data"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    auto& data = world.new_object() = As<Data<type>>{"data", {"1.0", "2.0*line", "data(0)+data(1)"}};

    expect(bool(data)) << [&]{ return data.error(); } << fatal;
    expect(bool(data({0}))) << [&]{ return data({0}).error(); } << fatal;
    expect(bool(data({1}))) << [&]{ return data({1}).error(); } << fatal;
    expect(bool(data({2}))) << [&]{ return data({2}).error(); } << fatal;

    expect(*data({0}) == 1.0_d);
    expect(*data({1}) == 2.0_d);
    expect(*data({2}) == 3.0_d);

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "unexpected name expressions"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    auto& data = world.new_object();

    data = As<Data<type>>{"cos+1", {}};
    expect(not bool(data)) << fatal;
    expect(data.error() == zc::Error::unexpected(Token::Add("+", 3), "cos+1")) << data.error() << fatal;

    data = As<Data<type>>{"cos(x)", {}};
    expect(not bool(data)) << fatal;
    expect(data.error() == zc::Error::name_already_taken("cos")) << data.error() << fatal;

    data = As<Data<type>>{"cos", {}};
    expect(not bool(data)) << fatal;
    expect(data.error() == zc::Error::name_already_taken("cos")) << data.error() << fatal;

    data = As<Data<type>>{"data(x,y,z)", {}};
    expect(not bool(data)) << fatal;
    expect(data.error() == zc::Error::unexpected(Token::Variable("y", 7), "data(x,y,z)")) << data.error() << fatal;

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "dependent expressions"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;

    auto& f = world.new_object() = "f(x) = x*data(2)";
    auto& val = world.new_object() = "val = 2*data(1)";
    auto& data = world.new_object() = As<Data<type>>{"data", {"1.0", "2.0*g(line)", "data(0)+data(1)+g(line)"}};

    expect(bool(data)) << [&]{ return data.error(); } << fatal;
    expect(bool(data({0}))) << [&]{ return data({0}).error(); } << fatal;
    expect(*data({0}) == 1.0_d) << fatal;

    expect(not bool(data({1}))) << fatal;
    expect(data({1}).error()
           == zc::Error::undefined_function(Token::Function("g", 4), "2.0*g(line)"))
      << data({1}).error() << fatal;

    expect(not bool(data({2}))) << fatal;
    expect(data({2}).error()
           == zc::Error::undefined_function(Token::Function("g", 16), "data(0)+data(1)+g(line)"))
      << data({2}).error() << fatal;

    auto& g = world.new_object() = "g(x) = 2*x";

    expect(bool(f)) << [&]{ return data.error(); } << fatal;
    expect(bool(val)) << [&]{ return val.error(); } << fatal;
    expect(bool(data)) << [&]{ return data.error(); } << fatal;
    expect(bool(g)) << [&]{ return g.error(); } << fatal;

    expect(bool(f({2}))) << [&]{ return f({2}).error(); } << fatal;
    expect(bool(val())) << [&]{ return val().error(); } << fatal;
    expect(bool(data({2}))) << [&]{ return data({2}).error(); } << fatal;
    expect(bool(g({2}))) << [&]{ return g({2}).error(); } << fatal;

    expect(*f({2}) == 18.0_d);
    expect(*val() == 8.0_d);
    expect(*data({2}) == 9.0_d);
    expect(*g({2}) == 4.0_d);

  } | std::tuple<FAST_TEST, RPN_TEST>{};
}
