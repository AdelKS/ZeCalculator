#pragma once

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

#include <memory>
#include <variant>
#include <vector>
#include <zecalculator/parsing/data_structures/decl/shared.h>
#include <zecalculator/parsing/types.h>
#include <zecalculator/utils/non_unique_ptr.h>
#include <zecalculator/utils/utils.h>

namespace zc {
namespace parsing {
  namespace ast {
    namespace node {

      template <parsing::Type, size_t>
      struct Function;

      template <parsing::Type world_type>
      using GlobalVariable = Function<world_type, 0>;

      template <parsing::Type>
      struct Sequence;

      template <parsing::Type, size_t args_num>
      struct CppFunction;

      template <parsing::Type, char op, size_t args_num>
      struct Operator;

      template <parsing::Type type, char op>
      using BinaryOperator = Operator<type, op, 2>;

      template <parsing::Type world_type>
      using Node = std::variant<shared::node::InputVariable,
                                shared::node::Number,
                                Operator<world_type, '=', 2>,
                                Operator<world_type, '+', 2>,
                                Operator<world_type, '-', 2>,
                                Operator<world_type, '*', 2>,
                                Operator<world_type, '/', 2>,
                                Operator<world_type, '^', 2>,
                                CppFunction<world_type, 1>,
                                CppFunction<world_type, 2>,
                                shared::node::GlobalConstant<world_type>,
                                Function<world_type, 0>,
                                Function<world_type, 1>,
                                Function<world_type, 2>,
                                Sequence<world_type>>;

      template <parsing::Type world_type>
      using NodePtr = zc::non_unique_ptr<Node<world_type>>;

    } // namespace node

  } // namespace ast

  /// @brief A tree representation in an AST or RPN world
  /// @note when the math world is RPN based, this AST is simply an intermediate form
  ///       before being transformed into an RPN representation
  template <parsing::Type world_type>
  using AST = ast::node::NodePtr<world_type>;

  } // namespace parsing
} // namespace zc
