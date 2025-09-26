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

#include <zecalculator/parsing/data_structures/token.h>
#include <zecalculator/zecalculator.h>

// testing specific headers
#include <boost/ut.hpp>
#include <zecalculator/test-utils/print-utils.h>
#include <zecalculator/test-utils/structs.h>
#include <zecalculator/test-utils/utils.h>

using namespace zc;
using namespace zc::parsing;

int main()
{
  using namespace boost::ut;
  using parsing::tokens::Text;

  "rename CppFunction"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;

    auto& cos_f = *world.get("cos");

    auto& f = world.new_object() = "f(x) = cos(x)";

    expect(bool(f));

    cos_f.set_name("better_cos");

    expect(cos_f.get_name() == "better_cos");

    expect(not bool(f)) << fatal;
    expect(f.error() == zc::Error::undefined_function(Text{"cos", 7}, "f(x) = cos(x)"))
      << f.error() << fatal;

    cos_f.set_name("cos");

    expect(bool(f));

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "rename Function"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;

    auto& f = world.new_object() = "  f(x) = cos(x)";
    auto& g = world.new_object() = "g(x) = f(x)+1";
    auto& data = world.new_object().set_data("data(line)", {"f(line)", "f(line)+1"});

    expect(bool(f)) << fatal;
    expect(bool(g)) << fatal;
    expect(bool(data) and bool(data({0})) and bool(data({1}))) << fatal;

    f.set_name("new_f(x)");

    expect(f.get_name() == "new_f");
    expect(f.get_equation() == "new_f(x)= cos(x)");

    expect(not bool(g)) << fatal;
    expect(not bool(data({0})) and not bool(data({1}))) << fatal;

    expect(g.error() == zc::Error::undefined_function(Text{"f", 7}, "g(x) = f(x)+1"))
      << g.error() << fatal;

    expect(data({1}).error() == zc::Error::undefined_function(Text{"f", 0}, "f(line)+1"))
      << data({1}).error() << fatal;

    f.set_name("f(x)");

    expect(bool(g));
    expect(bool(data({0})) and bool(data({1}))) << fatal;

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "rename Function without input vars"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;

    auto& f = world.new_object() = "  f(x) = cos(x)";

    expect(bool(f)) << fatal;

    f.set_name("new_f");

    expect(f.get_name() == "new_f");
    expect(f.get_equation() == "new_f= cos(x)");

    expect(not bool(f)) << fatal;
    expect(f.error() == zc::Error::undefined_variable(Text{"x", 11}, "new_f= cos(x)")) << f.error();

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "rename Data"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;

    auto& f = world.new_object() = "f(x) = data(0)";

    expect(not bool(f)) << fatal;

    auto& data = world.new_object().set_data("data", {"0", "1"});

    expect(bool(f)) << fatal;

    expect(f({0}).value() == 0.0_d) << fatal;

    data.set_name("better_data");

    expect(not bool(f)) << fatal;
    expect(f.error() == zc::Error::undefined_function(Text{"data", 7}, "f(x) = data(0)"))
      << f.error() << fatal;

    data.set_name("data");

    expect(bool(f)) << fatal;

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "fighting for the same name"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;

    auto& f1 = world.new_object() = "f(x) = 3*g(x)";

    expect(not bool(f1)) << fatal;

    auto& f2 = world.new_object() = "f(x) = 3*x+1";

    expect(not bool(f2) and f2.name_status().error().type == zc::Error::NAME_ALREADY_TAKEN)
      << fatal;

    auto& f3 = world.new_object() = "f(x) = 3*x+2";

    expect(not bool(f3) and f3.name_status().error().type == zc::Error::NAME_ALREADY_TAKEN)
      << fatal;

    f1.set_name("h(x)");

    expect(not bool(f1));
    expect(bool(f2));
    expect(not bool(f3) and f3.name_status().error().type == zc::Error::NAME_ALREADY_TAKEN);

    f2.set_name("g(x)");

    expect(bool(f1));
    expect(bool(f2));
    expect(bool(f3));


  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "revision updates"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;

    auto& f = world.new_object() = "f(x) = cos(x)+c";

    expect(f.get_revision() == 1_u) << fatal;

    auto& c = world.new_object().set("c", 1.);

    expect(f.get_revision() == 2_u) << fatal;

    c = 3.6;

    expect(f.get_revision() == 3_u) << fatal;

    auto& g = world.new_object() = "g(x) = f(x)+2+d";

    expect(g.get_revision() == 1_u) << fatal;

    auto& d = world.new_object().set("d", 1.);

    expect(g.get_revision() == 2_u) << fatal;

    auto& h = world.new_object() = "h(x) = g(x)+c";

    expect(h.get_revision() == 1_u) << fatal;

    c = 1.5;

    expect(f.get_revision() == 4_u) << fatal;
    expect(g.get_revision() == 3_u) << fatal;
    expect(h.get_revision() == 2_u) << fatal;

    d = 2.;

    expect(f.get_revision() == 4_u) << fatal;
    expect(g.get_revision() == 4_u) << fatal;
    expect(h.get_revision() == 3_u) << fatal;

    f = "f(x) = sin(x)+d";

    expect(f.get_revision() == 5_u) << fatal;
    expect(g.get_revision() == 5_u) << fatal;
    expect(h.get_revision() == 4_u) << fatal;

    d = 3.;

    expect(f.get_revision() == 6_u) << fatal;
    expect(g.get_revision() == 6_u) << fatal;
    expect(h.get_revision() == 5_u) << fatal;

    c = 2.;

    expect(f.get_revision() == 6_u) << fatal;
    expect(g.get_revision() == 6_u) << fatal;
    expect(h.get_revision() == 6_u) << fatal;

  } | std::tuple<FAST_TEST, RPN_TEST>{};


  "sequence & data revision updates"_test = []<class StructType>()
  {
    constexpr parsing::Type type = std::is_same_v<StructType, FAST_TEST> ? parsing::Type::FAST : parsing::Type::RPN;

    MathWorld<type> world;
    eval::Cache cache;

    auto& f = world.new_object().set_data("speed", {"c", "5", "7", "3", "2"});
    auto& g = world.new_object() = "g(n) = speed(0) ; speed(1)";
    auto& h = world.new_object() = "h(n) = g(0) ; g(1)";

    expect(f.get_revision() == 1_u) << fatal;
    expect(g.get_revision() == 1_u) << fatal;
    expect(h.get_revision() == 1_u) << fatal;

    auto& c = world.new_object().set("c", 1.);

    expect(f.get_revision() == 2_u) << fatal;
    expect(g.get_revision() == 2_u) << fatal;
    expect(h.get_revision() == 2_u) << fatal;

    expect(f({0.}, &cache).value() == 1._d);
    expect(h({0.}, &cache).value() == 1._d);
    expect(cache[f.get_slot()].get_cached_revision() == 2_u);
    expect(cache[g.get_slot()].get_cached_revision() == 2_u);
    expect(cache[h.get_slot()].get_cached_revision() == 2_u);

    c = 3.;

    expect(f.get_revision() == 3_u) << fatal;
    expect(f({0.}, &cache).value() == 3._d);
    expect(h({0.}, &cache).value() == 3._d);
    expect(cache[f.get_slot()].get_cached_revision() == 3_u);
    expect(cache[g.get_slot()].get_cached_revision() == 3_u);
    expect(cache[h.get_slot()].get_cached_revision() == 3_u);

  } | std::tuple<FAST_TEST, RPN_TEST>{};

}
