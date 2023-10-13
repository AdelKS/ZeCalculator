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

#include <memory>
#include <variant>
#include <vector>
#include <zecalculator/parsing/shared.h>
#include <zecalculator/utils/utils.h>

namespace zc {

struct SubstrInfo;

namespace parsing {
  namespace tokens {

    struct Variable;
    struct Number;
    struct Text;

  } // namespace tokens

  namespace node {

    struct InputVariable;
    struct GlobalConstant;

    using Number = zc::parsing::tokens::Number;

    namespace rpn {

      template <size_t>
      struct Function;

      struct Sequence;

      template <size_t>
      struct CppFunction;

      using Node = std::variant<std::monostate,
                                InputVariable,
                                Number,
                                CppFunction<1>,
                                CppFunction<2>,
                                GlobalConstant,
                                Function<0>,
                                Function<1>,
                                Function<2>,
                                Sequence>;

    } // namespace rpn

    namespace ast {

      template <parsing::Type, size_t>
      struct Function;

      template <parsing::Type world_type>
      using GlobalVariable = Function<world_type, 0>;

      template <parsing::Type>
      struct Sequence;

      template <parsing::Type, size_t args_num>
      struct CppFunction;

      template <parsing::Type world_type>
      using Node = std::variant<std::monostate,
                                InputVariable,
                                Number,
                                CppFunction<world_type, 1>,
                                CppFunction<world_type, 2>,
                                GlobalConstant,
                                Function<world_type, 0>,
                                Function<world_type, 1>,
                                Function<world_type, 2>,
                                Sequence<world_type>>;

      template <parsing::Type>
      struct NodePtr;

    } // namespace ast

  } // namespace node

  /// @brief A tree representation in an AST or RPN world
  /// @note when the math world is RPN based, this Tree is simply an intermediate form
  ///       before being transformed into an RPN representation
  template <parsing::Type world_type>
  using Tree = node::ast::NodePtr<world_type>;

  using RPN = std::vector<node::rpn::Node>;

  template <parsing::Type type>
  using Parsing = std::conditional_t<type == parsing::Type::AST, Tree<parsing::Type::AST>, RPN>;

  template <class NodeType>
    requires(is_any_of<NodeType,
                       node::ast::Node<parsing::Type::AST>,
                       node::ast::Node<parsing::Type::RPN>,
                       node::rpn::Node>)
  parsing::tokens::Text text_token(const NodeType& token);

  template <class NodeType>
    requires(is_any_of<NodeType,
                       node::ast::Node<parsing::Type::AST>,
                       node::ast::Node<parsing::Type::RPN>,
                       node::rpn::Node>)
  SubstrInfo substr_info(const NodeType& token);

  } // namespace parsing
} // namespace zc
