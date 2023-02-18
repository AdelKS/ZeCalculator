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

#include <zecalculator/utils/parser.h>
#include <zecalculator/utils/syntax_tree.h>

// testing specific headers
#include <boost/ut.hpp>
#include <zecalculator/test-utils/print-utils.h>

using namespace zc;

int main()
{
  using namespace boost::ut;

  "simple expression"_test = []()
  {
    auto parsing = parse("2+2*2");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_tree(parsing.value());

    expect(bool(expect_node));

    SyntaxTree expected_node = SyntaxTree {
      .type = SyntaxTree::FUNCTION,
      .str = "+",
      .subnodes = {
        SyntaxTree {
          .type = SyntaxTree::NUMBER,
          .str = "2",
          .value = 2.0
        },
        SyntaxTree {
          .type = SyntaxTree::FUNCTION,
          .str = "*",
          .subnodes = {
            SyntaxTree {
              .type = SyntaxTree::NUMBER,
              .str = "2",
              .value = 2.0
            },
            SyntaxTree {
              .type = SyntaxTree::NUMBER,
              .str = "2",
              .value = 2.0
            },
          }
        }
      }
    };

    expect(*expect_node == expected_node) << *expect_node;
  };

  "function expression"_test = []()
  {
    auto parsing = parse("(cos(sin(x)+1))+1");

    expect(bool(parsing)) << parsing;

    auto expect_node = make_tree(parsing.value());

    expect(bool(expect_node));

    SyntaxTree expected_node = SyntaxTree {
      .type = SyntaxTree::FUNCTION,
      .str = "+",
      .subnodes = {
        SyntaxTree {
          .type = SyntaxTree::FUNCTION,
          .str = "cos",
          .subnodes = {
            SyntaxTree {
              .type = SyntaxTree::FUNCTION,
              .str = "+",
              .subnodes = {
                SyntaxTree {
                  .type = SyntaxTree::FUNCTION,
                  .str = "sin",
                  .subnodes = {
                    SyntaxTree {
                      .type = SyntaxTree::VARIABLE,
                      .str = "x"
                    },
                  }
                },
                SyntaxTree {
                  .type = SyntaxTree::NUMBER,
                  .str = "1",
                  .value = 1.0
                },
              },
            }
          }
        },
        SyntaxTree {
          .type = SyntaxTree::NUMBER,
          .str = "1",
          .value = 1.0
        },
      }
    };

    expect(*expect_node == expected_node) << expect_node;
  };
}
