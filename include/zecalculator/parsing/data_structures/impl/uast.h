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
#include <zecalculator/parsing/data_structures/decl/uast.h>
#include <zecalculator/parsing/data_structures/impl/shared.h>
#include <zecalculator/parsing/data_structures/token.h>

namespace zc {
  namespace parsing {
    namespace uast {
      namespace node {

        template <char op, size_t args_num>
        struct Operator: parsing::tokens::Text
        {
          using Operands = std::array<NodePtr, args_num>;

          Operator(parsing::tokens::Text text, parsing::tokens::Text name_token, Operands operands)
            : parsing::tokens::Text(std::move(text)), name_token(std::move(name_token)),
              operands(std::move(operands)){};

          parsing::tokens::Text name_token;
          Operands operands;
        };

        struct Function: parsing::tokens::Text
        {
          Function(parsing::tokens::Text full_expr,
                   parsing::tokens::Text name_token,
                   parsing::tokens::Text args_token,
                   std::vector<NodePtr> subnodes)
            : parsing::tokens::Text(std::move(full_expr)), name_token(std::move(name_token)),
              args_token(std::move(args_token)), subnodes(std::move(subnodes))
          {}

          parsing::tokens::Text name_token;
          parsing::tokens::Text args_token;
          std::vector<NodePtr> subnodes;

          bool operator == (const Function& other) const = default;
        };

      } // namespace node
    } // namespace uast
  } // namespace parsing
} // namespace zc
