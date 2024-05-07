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

#include <zecalculator/mathworld/mathworld.h>

// testing specific headers
#include <boost/ut.hpp>
#include <zecalculator/test-utils/print-utils.h>
#include <zecalculator/test-utils/structs.h>

using namespace zc;

int main()
{
  using namespace boost::ut;

  "simple test"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    CppUnaryFunction<type>* cppFunc = world.template get<CppUnaryFunction<type>>("sqrt");
    expect(cppFunc != nullptr and (*cppFunc)(4) == 2);

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "Add constant then set value"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    auto& c1 = world.add("my_constant1 = 0").template value_as<GlobalConstant<type>>();

    c1 = 2.0;
    expect(c1 == 2.0);

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "Add same constant twice"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    world.add("my_constant1 = 2.0");
    expect(not world.add("my_constant1 = 3.0"));

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "erase object with pointer"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    auto& f = world.add("f(x) = cos(x)");
    auto& g = world.add("g(x) = f(x)+1");

    expect(f.has_value()) << [&]{ return f.error(); } << fatal;
    expect(g.has_value()) << [&]{ return g.error(); } << fatal;

    expect(bool(world.erase(f)));

    // cannot erase the same object twice
    expect(not bool(world.erase(f)));

    // after erasing 'f', 'g' must have been re-parsed and get an undefined function error on f
    expect(not g.has_value()) << fatal;
    expect(g.error().type == Error::UNDEFINED_FUNCTION
           and g.error().token.substr == "f") << g.error();

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "invalidity chain"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    auto& f = world.add("f(x) = g(x)+1");

    expect(not f.has_value()) << fatal;
    expect(f.error().type == Error::UNDEFINED_FUNCTION
           and f.error().token == parsing::tokens::Text{.substr = "g", .begin = 7});

    auto& g = world.add("g(x) = z(x)+1");

    expect(not f.has_value()) << fatal;
    expect(f.error().type == Error::OBJECT_INVALID_STATE
           and f.error().token == parsing::tokens::Text{.substr = "g", .begin = 7});

    expect(not g.has_value()) << fatal;
    expect(g.error().type == Error::UNDEFINED_FUNCTION
           and g.error().token == parsing::tokens::Text{.substr = "z", .begin = 7});

    auto& z = world.add("z(x) = f(x)+1");

    auto res = z(1);
    expect(not res and res.error() == Error::recursion_depth_overflow());

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "cannot erase object in the wrong world"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world1;
    world1.add("f(x) = cos(x)");

    MathWorld<type> world2;
    auto& g = world2.template add<Function<type>>("g(x) = sin(x)+1");

    // cannot erase 'g' in 'world1'
    expect(not bool(world1.erase(g)));

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "erase object by name"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    auto& f = world.add("f(x)=cos(x)");
    auto& g = world.add("g(x)= f(x)+1");

    // no issues expected with parsing of 'f' nor 'g'
    expect(f.has_value()) << [&]{ return f.error(); } << fatal;
    expect(g.has_value()) << [&]{ return g.error(); } << fatal;

    // "cos" is part of the default constructed math world
    expect(bool(world.erase("cos")));

    // cannot erase the same object twice
    expect(not bool(world.erase("cos")));

    // after erasing 'cos', 'f' must have been reparsed and get an undefined function error on 'cos
    expect(not f.has_value() and f.error().type == Error::UNDEFINED_FUNCTION and f.error().token.substr == "cos");

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  return 0;
}
