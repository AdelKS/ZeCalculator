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
using parsing::tokens::Text;

int main()
{
  using namespace boost::ut;

  "simple test"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    CppUnaryFunction* cppFunc = world.template get<CppUnaryFunction>("sqrt");
    expect(cppFunc != nullptr and (*cppFunc)({4}) == 2);

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "Add constant then set value"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    auto& c1 = (world.new_object() = "my_constant1 = 42").template value_as<GlobalConstant>();

    expect(c1.value == 42.0);

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "Add same constant twice"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    world.new_object() = "my_constant1 = 2.0";
    expect(not (world.new_object() = "my_constant1 = 3.0"));

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "Add constant with white spaces 1"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    auto& cst = world.new_object() = "   my_constant1 = 2.0";
    expect(cst.get_name() == "my_constant1");

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "Add constant with white spaces 2"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    auto& cst = world.new_object().set("  cst   ", {1.0});
    expect(cst.get_name() == "cst");

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "Add CppFunction with white spaces"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    auto& f = world.new_object().set(" better_cos   ", CppFunction<1>{std::cos});
    expect(f.get_name() == "better_cos");

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "Add CppFunction with invalid name"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    auto& f = world.new_object().set(" 1+1   ", CppFunction<1>{std::cos});
    expect(f.error() == zc::Error::unexpected(Text{"+", 2}, " 1+1   ")) << f.error();

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "erase object with pointer"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    auto& f = world.new_object() = "f(x) = cos(x)";
    auto& g = world.new_object() = "g(x) = f(x)+1";

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
    auto& f = world.new_object() = "f(x) = g(x)+1";

    expect(not f.has_value()) << fatal;
    expect(f.error().type == Error::UNDEFINED_FUNCTION
           and f.error().token == parsing::tokens::Text{.substr = "g", .begin = 7});

    auto& g = world.new_object() = "g(x) = z(x)+1";

    expect(not f.has_value()) << fatal;
    expect(f.error().type == Error::OBJECT_INVALID_STATE
           and f.error().token == parsing::tokens::Text{.substr = "g", .begin = 7});

    expect(not g.has_value()) << fatal;
    expect(g.error().type == Error::UNDEFINED_FUNCTION
           and g.error().token == parsing::tokens::Text{.substr = "z", .begin = 7});

    auto& z = world.new_object() = "z(x) = f(x)+1";

    expect(f.has_value()) << [&]{ return f.error(); } << fatal;
    expect(g.has_value()) << [&]{ return g.error(); } << fatal;
    expect(z.has_value()) << [&]{ return z.error(); } << fatal;

    auto res = z({1});
    expect(not res and res.error() == Error::recursion_depth_overflow()) << res;

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "cannot erase object in the wrong world"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world1;
    world1.new_object() = "f(x) = cos(x)";

    MathWorld<type> world2;
    auto& g = world2.new_object() = As<Function<type>>{"g(x) = sin(x)+1"};

    // cannot erase 'g' in 'world1'
    expect(not bool(world1.erase(g)));

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "erase object by name"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    auto& f = world.new_object() = "f(x)=cos(x)";
    auto& g = world.new_object() = "g(x)= f(x)+1";

    // no issues expected with parsing of 'f' nor 'g'
    expect(f.has_value()) << [&]{ return f.error(); } << fatal;
    expect(g.has_value()) << [&]{ return g.error(); } << fatal;

    // "cos" is part of the default constructed math world
    expect(bool(world.erase("cos")));

    // cannot erase the same object twice
    expect(not bool(world.erase("cos")));

    // after erasing 'cos', 'f' must have been re-parsed and get an undefined function error on 'cos
    expect(not f.has_value() and f.error().type == Error::UNDEFINED_FUNCTION and f.error().token.substr == "cos");

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  return 0;
}
