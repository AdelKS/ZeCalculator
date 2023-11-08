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
#include <zecalculator/parsing/data_structures/decl/utils.h>
#include <zecalculator/parsing/data_structures/impl/ast.h>
#include <zecalculator/parsing/data_structures/impl/rpn.h>
#include <zecalculator/parsing/data_structures/token.h>

namespace zc {
  namespace parsing {

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