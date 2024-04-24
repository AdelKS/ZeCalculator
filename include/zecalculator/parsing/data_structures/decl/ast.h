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

#include <variant>
#include <vector>

#include <zecalculator/parsing/data_structures/decl/shared.h>
#include <zecalculator/parsing/data_structures/token.h>
#include <zecalculator/utils/utils.h>

namespace zc {
  namespace parsing {
    namespace ast {

      struct Node {

        /// @brief will represent both functions and (infix) operators
        struct Func
        {
          enum Type {
            FUNCTION = tokens::FUNCTION,
            OP_ASSIGN = tokens::OP_ASSIGN,
            OP_ADD = tokens::OP_ADD,
            OP_SUBTRACT = tokens::OP_SUBTRACT,
            OP_MULTIPLY = tokens::OP_MULTIPLY,
            OP_DIVIDE = tokens::OP_DIVIDE,
            OP_POWER = tokens::OP_POWER,
          };

          Type type;

          /// @brief full expression of the operation of the function/operation,
          ///        including arguments/operands
          parsing::tokens::Text full_expr;

          /// @brief arguments to operator or to function
          std::vector<Node> subnodes;

          bool operator == (const Func&) const = default;
        };

        struct InputVariable {
          /// @brief index of input variable
          size_t index;

          bool operator == (const InputVariable&) const = default;
        };

        struct Number {
          /// @brief value if a number
          double value;

          bool operator == (const Number&) const = default;
        };

        struct Variable {
          bool operator == (const Variable&) const { return true; }
        };

        /// @brief full expression of the node, including its children
        parsing::tokens::Text name;

        /// @brief data that depends on the type of the node
        std::variant<Variable, InputVariable, Number, Func> dyn_data = {};

        static Node make_func(Func::Type type,
                              parsing::tokens::Text name,
                              parsing::tokens::Text full_expr,
                              std::vector<Node> subnodes);
        static Node make_input_var(parsing::tokens::Text name, size_t index);
        static Node make_number(parsing::tokens::Text name, double value);
        static Node make_var(parsing::tokens::Text name);

        bool is_func() const;
        bool is_input_var() const;
        bool is_number() const;
        bool is_var() const;

        const Func& func_data() const;
        const InputVariable& input_var_data() const;
        const Number& number_data() const;

        Func& func_data();
        InputVariable& input_var_data();
        Number& number_data();

        tokens::Text args_token() const;

        bool operator == (const Node&) const = default;
      };

    } // namespace ast

    /// @brief Unbound AST : a generic tree that only keeps string names of objects
    using AST = ast::Node;

  } // namespace parsing
} // namespace zc
