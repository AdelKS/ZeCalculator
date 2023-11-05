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
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    CppUnaryFunction<type>* cppFunc = world.template get<CppUnaryFunction<type>>("sqrt");
    expect(cppFunc != nullptr and (*cppFunc)(4) == 2);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "Add constant then set value"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    GlobalConstant<type>& c1 = world.template add<GlobalConstant<type>>("my_constant1").value();
    c1.value = 2.0;
    expect(c1.value == 2.0);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "Add same constant twice"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    world.template add<GlobalConstant<type>>("my_constant1", 2.0);
    expect(not world.template add<GlobalConstant<type>>("my_constant1", 3.0).has_value());

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "Rename inexistent object"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    auto res = world.rename("foo", "bar");
    expect(not res and res.error().type == Error::OBJECT_NOT_IN_WORLD);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "Rename object with deps"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    world.template add<Function<type, 1>>("f", Vars<1>{"x"}, "cos(x)");

    expect(bool(world.rename("cos", "couscous")));

    auto res = world.evaluate("f(1)");
    expect(not res and res.error().type == Error::UNDEFINED_FUNCTION
           and res.error().token.name == "cos") << res;

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "set name"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    Function<type, 1>& f = world.template add<Function<type, 1>>("f", Vars<1>{"x"}, "cos(x)").value();

    expect(bool(world.set_name(&f, "g")));

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "set name affects deps"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    Function<type, 1>& f = world.template add<Function<type, 1>>("f", Vars<1>{"x"}, "cos(x)").value();
    Function<type, 1>& g = world.template add<Function<type, 1>>("g", Vars<1>{"x"}, "f(x)+1").value();

    expect(bool(world.set_name(&f, "h")));
    expect(g.error()->type == Error::UNDEFINED_FUNCTION and g.error()->token.name == "f") << g.error();

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "set name already taken"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    Function<type, 1>& f = world.template add<Function<type, 1>>("f", Vars<1>{"x"}, "cos(x)").value();

    auto res = world.set_name(&f, "cos");
    expect(not res and res.error().type == Error::NAME_ALREADY_TAKEN);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "set name invalid format"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    Function<type, 1>& f = world.template add<Function<type, 1>>("f", Vars<1>{"x"}, "cos(x)").value();

    auto res = world.set_name(&f, "1+1");
    expect(not res and res.error().type == Error::WRONG_FORMAT);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "set name not in world"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world1, world2;
    Function<type, 1>& f = world1.template add<Function<type, 1>>("f", Vars<1>{"x"}, "cos(x)").value();

    // try to change the name of object in the wrong world
    auto res = world2.set_name(&f, "g");
    expect(not res and res.error().type == Error::OBJECT_NOT_IN_WORLD);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "add nameless math object"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    Function<type, 1>& f = world.template add<Function<type, 1>>();

    Function<type, 1>& g = world.template add<Function<type, 1>>("g", Vars<1>{"x"}, "f(x)+1").value();

    expect(bool(g.error()) and g.error()->type == Error::UNDEFINED_FUNCTION
           and g.error()->token.name == "f") << g.error();

    // give a name to "f"
    auto status = world.set_name(&f, "f");
    expect(bool(status)) << status;

    // now 'g' should be in a valid state
    expect(not bool(g.error())) << g.error();

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "erase object with pointer"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    Function<type, 1>& f = world.template add<Function<type, 1>>("f", Vars<1>{"x"}, "cos(x)").value();
    Function<type, 1>& g = world.template add<Function<type, 1>>("g", Vars<1>{"x"}, "f(x)+1").value();

    // no issues expected with parsing of 'f' nor 'g'
    expect(not bool(f.error()));
    expect(not bool(g.error()));

    expect(bool(world.erase(&f)));

    // cannot erase the same object twice
    expect(not bool(world.erase(&f)));

    // after erasing 'f', 'g' must have been reparsed and get an undefined function error on f
    expect(bool(g.error()) and g.error()->type == Error::UNDEFINED_FUNCTION and g.error()->token.name == "f");

    // add a new function
    Function<type, 1>& h = world.template add<Function<type, 1>>("h", Vars<1>{"x"}, "1+x").value();

    // no issues expected with 'h'
    expect(not bool(h.error()));

    // 'h' must be built where 'f' was built before
    expect(&h == &f);

  } | std::tuple<AST_TEST, RPN_TEST>{};

  "erase object by name"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, AST_TEST> ? parsing::Type::AST : parsing::Type::RPN;

    MathWorld<type> world;
    Function<type, 1>& f = world.template add<Function<type, 1>>("f", Vars<1>{"x"}, "cos(x)").value();
    Function<type, 1>& g = world.template add<Function<type, 1>>("g", Vars<1>{"x"}, "f(x)+1").value();

    // no issues expected with parsing of 'f' nor 'g'
    expect(not bool(f.error()));
    expect(not bool(g.error()));

    // "cos" is part of the default constructed math world
    expect(bool(world.erase("cos")));

    // cannot erase the same object twice
    expect(not bool(world.erase("cos")));

    // after erasing 'cos', 'f' must have been reparsed and get an undefined function error on 'cos
    expect(bool(f.error()) and f.error()->type == Error::UNDEFINED_FUNCTION and f.error()->token.name == "cos");

  } | std::tuple<AST_TEST, RPN_TEST>{};

  return 0;
}
