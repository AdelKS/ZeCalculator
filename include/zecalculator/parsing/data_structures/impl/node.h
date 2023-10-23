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

#include <zecalculator/math_objects/aliases.h>
#include <zecalculator/math_objects/decl/function.h>
#include <zecalculator/math_objects/decl/sequence.h>
#include <zecalculator/math_objects/global_constant.h>
#include <zecalculator/math_objects/cpp_function.h>
#include <zecalculator/parsing/data_structures/decl/node.h>
#include <zecalculator/parsing/data_structures/token.h>

namespace zc {
  namespace parsing {
    namespace node {

      using zc::parsing::tokens::Text;

      struct InputVariable: Text
      {
        InputVariable(const Text& txt, size_t index)
          : Text(txt), index(index) {}

        size_t index;
      };

      template <parsing::Type world_type>
      struct GlobalConstant: Text
      {
        GlobalConstant(const Text& txt, const zc::GlobalConstant<world_type>* constant)
          : Text(txt), constant(constant) {}

        const zc::GlobalConstant<world_type>* constant;
      };

      namespace rpn {

        using parsing::Type::RPN;

        template <size_t args_num>
        struct Function: Text
        {
          Function(const Text& txt, const zc::Function<RPN, args_num>* f)
            : Text(txt),  f(f) {}

          const zc::Function<RPN, args_num>* f;
        };

        struct Sequence: Text
        {
          Sequence(const Text& txt, const zc::Sequence<RPN>* u)
            : Text(txt),  u(u) {}

          const zc::Sequence<RPN>* u;
        };

        template <size_t args_num>
        struct CppFunction: Text
        {
          CppFunction(const Text& txt, const zc::rpn::CppFunction<args_num>* f)
            : Text(txt),  f(f) {}

          const zc::rpn::CppFunction<args_num>* f;
        };

        template <char op, size_t args_num>
        struct Operator: zc::parsing::tokens::Operator<op, args_num>
        {
          using Parent = zc::parsing::tokens::Operator<op, args_num>;

          Operator(const Parent& op_token): Parent(op_token) {}
          Operator(size_t begin) : zc::parsing::tokens::Operator<op, args_num>(begin){};

          Operator(const Operator&) = default;
        };

      } // namespace rpn

      namespace ast {

        using parsing::Type::AST;

        template <parsing::Type world_type>
        struct NodePtr: std::unique_ptr<Node<world_type>>
        {
          using Parent = std::unique_ptr<Node<world_type>>;

          NodePtr() : Parent(std::make_unique<Node<world_type>>()) {}

          template <class T>
            requires(not std::is_same_v<std::remove_cvref_t<T>, NodePtr>
                     and std::is_convertible_v<T, Node<world_type>>)
          NodePtr(T&& val) : Parent(std::make_unique<Node<world_type>>(std::forward<T>(val)))
          {}

          NodePtr(Parent&& parent_rval)
            : Parent(std::move(parent_rval))
          {}

          NodePtr(NodePtr&&) = default;
          NodePtr& operator = (NodePtr&&) = default;

          bool operator == (const NodePtr& other) const
          {
            assert(*this and other); // cannot be nullptrs, invariant of this class
            return **this == *other;
          }
        };

        template <parsing::Type world_type, size_t args_num>
        struct Function: Text
        {
          using Operands = std::array<NodePtr<world_type>, args_num>;

          Function(const Text& txt, const zc::Function<world_type, args_num>* f, Operands operands)
            : Text(txt), f(f), operands(std::move(operands)) {}

          Function(const Text& txt, const zc::Function<world_type, args_num>* f)
            requires (args_num == 0)
            : Text(txt), f(f) {}

          const zc::Function<world_type, args_num>* f;
          Operands operands;
        };

        template <parsing::Type world_type>
        struct Sequence: Text
        {
          Sequence(const Text& txt, const zc::Sequence<world_type>* u, NodePtr<world_type> operand)
            : Text(txt), u(u), operand(std::move(operand)) {}

          const zc::Sequence<world_type>* u;
          NodePtr<world_type> operand;
        };

        template <parsing::Type world_type, size_t args_num>
        struct CppFunction: Text
        {
          CppFunction(const Text& txt,
                      const zc::CppFunction<world_type, args_num>* f,
                      std::array<NodePtr<world_type>, args_num> operands)
            : Text(txt), f(f), operands(std::move(operands))
          {}

          const zc::CppFunction<world_type, args_num>* f;
          std::array<NodePtr<world_type>, args_num> operands;
        };

        template <parsing::Type world_type, char op, size_t args_num>
        struct Operator: rpn::Operator<op, args_num>
        {
          using Parent = rpn::Operator<op, args_num>;

          Operator(const Parent& parent, std::array<NodePtr<world_type>, args_num> operands)
            : Parent(parent), operands(std::move(operands)){};

          std::array<NodePtr<world_type>, args_num> operands;
        };

      } // namespace rpn
    } // namespace node

    template <class NodeType>
      requires(utils::is_any_of<NodeType,
                                node::ast::Node<parsing::Type::AST>,
                                node::ast::Node<parsing::Type::RPN>,
                                node::rpn::Node>)
    inline parsing::tokens::Text text_token(const NodeType& token)
    {
      return std::visit(
        utils::overloaded {
          [](const std::monostate&) -> parsing::tokens::Text {
            return parsing::tokens::Text();
          },
          [](const auto& tk) -> parsing::tokens::Text {
            return tk;
          }},
        token);
    }

    template <class NodeType>
      requires(utils::is_any_of<NodeType,
                                node::ast::Node<parsing::Type::AST>,
                                node::ast::Node<parsing::Type::RPN>,
                                node::rpn::Node>)
    inline SubstrInfo substr_info(const NodeType& token)
    {
      return text_token(token).substr_info;

    }

  } // namespace parsing
} // namespace zc
