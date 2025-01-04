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
#include <zecalculator/utils/variant.h>

using namespace zc::parsing;

int main()
{
  using namespace boost::ut;

  "simple expression"_test = []<class StructType>()
  {
    constexpr Type type = std::is_same_v<StructType, FAST_TEST> ? Type::FAST : Type::RPN;

    zc::MathWorld<type> world;
    std::string expression = "2+2*2";

    auto expect_node = tokenize(expression)
                         .and_then(make_ast{expression})
                         .transform(flatten_separators)
                         .and_then(make_fast<type>{expression, world});

    expect(bool(expect_node)) << expect_node << fatal;

    using T = FAST<type>;

    auto expected_node = T{shared::node::Add{},
                           {T{shared::node::Number{2.0}},
                            {T{shared::node::Multiply{},
                                 {T{shared::node::Number{2.0}}, T{shared::node::Number{2.0}}}}}}};

    expect(*expect_node == expected_node) << *expect_node;

  } | std::tuple<FAST_TEST, RPN_TEST>{};

  "function expression"_test = []<class StructType>()
  {
    constexpr Type type = std::is_same_v<StructType, FAST_TEST> ? Type::FAST : Type::RPN;
    zc::MathWorld<type> world;

    std::string expression = "(cos(sin(x)+1))+1";

    auto expect_node = tokenize(expression)
                         .and_then(make_ast{expression, std::array{"x"}})
                         .transform(flatten_separators)
                         .and_then(make_fast<type>{expression, world});

    expect(bool(expect_node)) << expect_node << fatal;

    using T = FAST<type>;
    using Node = shared::Node<type>;
    using zc::utils::variant_convert;

    FAST<type> expected_node = T{shared::node::Add{},
                                 {T{variant_convert<Node>{}(*world.get("cos")->get_linked_repr()),
                                    {T{shared::node::Add{},
                                       {T{variant_convert<Node>{}(*world.get("sin")->get_linked_repr()),
                                          {T{shared::node::InputVariable{0}}}},
                                        T{shared::node::Number{1.0}}}}}},
                                  T{shared::node::Number{1.0}}}};

    expect(*expect_node == expected_node) << *expect_node;

  } | std::tuple<FAST_TEST, RPN_TEST>{};
}
