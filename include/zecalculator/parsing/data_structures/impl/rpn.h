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
#include <zecalculator/parsing/data_structures/decl/fast.h>
#include <zecalculator/parsing/data_structures/decl/rpn.h>
#include <zecalculator/parsing/data_structures/impl/shared.h>
#include <zecalculator/parsing/data_structures/token.h>

namespace zc {
  namespace parsing {
    namespace rpn {

      namespace node {

        using parsing::Type::RPN;
        using tokens::Text;

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
        struct Operator: Text
        {
          Operator(const Text& txt): Text(txt) {}

          Operator(const Operator&) = default;
        };

      } // namespace rpn

    } // namespace node

  } // namespace parsing
} // namespace zc
