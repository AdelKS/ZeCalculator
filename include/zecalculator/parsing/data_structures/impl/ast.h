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

#include <utility>
#include <variant>
#include <zecalculator/math_objects/aliases.h>
#include <zecalculator/parsing/data_structures/decl/ast.h>
#include <zecalculator/parsing/data_structures/impl/shared.h>
#include <zecalculator/parsing/data_structures/token.h>

namespace zc {
  namespace parsing {
    namespace ast {

      inline const Node::Func& Node::func_data() const
      {
        assert(std::holds_alternative<Node::Func>(dyn_data));
        return *std::get_if<Node::Func>(&dyn_data);
      }

      inline const Node::InputVariable& Node::input_var_data() const
      {
        assert(std::holds_alternative<Node::InputVariable>(dyn_data));
        return *std::get_if<Node::InputVariable>(&dyn_data);
      }

      inline const Node::Number& Node::number_data() const
      {
        assert(std::holds_alternative<Node::Number>(dyn_data));
        return *std::get_if<Node::Number>(&dyn_data);
      }

      inline Node::Func& Node::func_data()
      {
        return const_cast<Func&>(std::as_const(*this).func_data());
      }

      inline Node::InputVariable& Node::input_var_data()
      {
        return const_cast<InputVariable&>(std::as_const(*this).input_var_data());
      }

      inline Node::Number& Node::number_data()
      {
        return const_cast<Number&>(std::as_const(*this).number_data());
      }

      inline Node Node::make_func(Node::Func::Type type,
                              parsing::tokens::Text name,
                              parsing::tokens::Text full_expr,
                              std::vector<Node> subnodes)
      {
        return Node{.name = std::move(name),
                    .dyn_data = Func{.type = type,
                                     .full_expr = std::move(full_expr),
                                     .subnodes = std::move(subnodes)}};
      }

      inline Node Node::make_input_var(parsing::tokens::Text name, size_t index)
      {
        return Node{.name = name, .dyn_data = InputVariable{index}};
      }

      inline Node Node::make_number(parsing::tokens::Text name, double value)
      {
        return Node{.name = name, .dyn_data = Number{value}};
      }

      inline Node Node::make_var(parsing::tokens::Text name)
      {
        return Node{.name = name};
      }

      inline tokens::Text Node::args_token() const
      {
        assert(std::holds_alternative<Func>(dyn_data) and func_data().type == Func::FUNCTION);

        tokens::Text args_token;
        // remove the function name and the opening parenthesis and the last parenthesis
        args_token.substr_info = SubstrInfo{.begin = func_data().full_expr.substr_info->begin
                                                     + name.substr.size() + 1,
                                            .size = func_data().full_expr.substr_info->size
                                                    - name.substr.size() - 2};
        args_token.substr = func_data().full_expr.substr.substr(name.substr.size() + 1,
                                                                args_token.substr_info->size);
        return args_token;
      }

      inline bool Node::is_func() const
      {
        return std::holds_alternative<Func>(dyn_data);
      }

      inline bool Node::is_input_var() const
      {
        return std::holds_alternative<InputVariable>(dyn_data);
      }

      inline bool Node::is_number() const
      {
        return std::holds_alternative<Number>(dyn_data);
      }

      inline bool Node::is_var() const
      {
        return std::holds_alternative<Variable>(dyn_data);
      }

    } // namespace ast
  } // namespace parsing
} // namespace zc
