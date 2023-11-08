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
#include <zecalculator/math_objects/cpp_function.h>
#include <zecalculator/math_objects/decl/function.h>
#include <zecalculator/math_objects/decl/sequence.h>
#include <zecalculator/math_objects/global_constant.h>
#include <zecalculator/parsing/data_structures/decl/ast.h>
#include <zecalculator/parsing/data_structures/impl/shared.h>
#include <zecalculator/parsing/data_structures/token.h>

namespace zc {
  namespace parsing {
    namespace node {
      namespace ast {

        template <parsing::Type world_type>
        struct NodePtr: std::unique_ptr<Node<world_type>>
        {
          using Parent = std::unique_ptr<Node<world_type>>;

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
        struct Function: tokens::Text
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
        struct Sequence: tokens::Text
        {
          Sequence(const Text& txt, const zc::Sequence<world_type>* u, NodePtr<world_type> operand)
            : Text(txt), u(u), operand(std::move(operand)) {}

          const zc::Sequence<world_type>* u;
          NodePtr<world_type> operand;
        };

        template <parsing::Type world_type, size_t args_num>
        struct CppFunction: tokens::Text
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
        struct Operator: zc::parsing::tokens::Operator<op, args_num>
        {
          using Parent = zc::parsing::tokens::Operator<op, args_num>;

          Operator(const Parent& parent, std::array<NodePtr<world_type>, args_num> operands)
            : Parent(parent), operands(std::move(operands)){};

          std::array<NodePtr<world_type>, args_num> operands;
        };

      } // namespace ast
    } // namespace node
  } // namespace parsing
} // namespace zc