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

#pragma once

#include <optional>
#include <span>
#include <vector>

#include <zecalculator/parsing/data_structures/decl/shared.h>
#include <zecalculator/parsing/data_structures/token.h>
#include <zecalculator/utils/utils.h>
#include <zecalculator/utils/non_unique_ptr.h>

namespace zc {
namespace parsing {
  namespace ast {
    namespace node {

      struct Function;

      template <char op, size_t args_num>
      struct Operator;

      template <char op>
      using BinaryOperator = Operator<op, 2>;

      using Variable = parsing::tokens::Variable;

      using Node = std::variant<shared::node::Number,
                                shared::node::InputVariable,
                                ast::node::Function,
                                ast::node::Variable,
                                ast::node::Operator<'=', 2>,
                                ast::node::Operator<'+', 2>,
                                ast::node::Operator<'-', 2>,
                                ast::node::Operator<'*', 2>,
                                ast::node::Operator<'/', 2>,
                                ast::node::Operator<'^', 2>>;

      using NodePtr = zc::non_unique_ptr<Node>;

      } // namespace ast
    } // namespace node

    /// @brief Unbound AST : a generic tree that only keeps string names of objects
    using AST = ast::node::NodePtr;

  } // namespace parsing
} // namespace zc
