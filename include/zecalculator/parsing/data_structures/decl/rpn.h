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
#include <zecalculator/parsing/data_structures/decl/shared.h>
#include <zecalculator/parsing/types.h>
#include <zecalculator/utils/utils.h>

namespace zc {
namespace parsing {
  namespace rpn {
    namespace node {

      template <size_t>
      struct Function;

      struct Sequence;

      template <size_t>
      struct CppFunction;

      template <char op, size_t args_num>
      struct Operator;

      template <char op>
      using BinaryOperator = Operator<op, 2>;

      using GlobalConstant = shared::node::GlobalConstant<parsing::Type::RPN>;

      using Node = std::variant<shared::node::InputVariable,
                                shared::node::Number,
                                Operator<'=', 2>,
                                Operator<'+', 2>,
                                Operator<'-', 2>,
                                Operator<'*', 2>,
                                Operator<'/', 2>,
                                Operator<'^', 2>,
                                CppFunction<1>,
                                CppFunction<2>,
                                GlobalConstant,
                                Function<0>,
                                Function<1>,
                                Function<2>,
                                Sequence>;

    } // namespace node

  } // namespace rpn

  using RPN = std::vector<rpn::node::Node>;

} // namespace parsing
} // namespace zc
