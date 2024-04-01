#pragma once

/****************************************************************************
**  Copyright (c) 2023, Adel Kara Slimane <adel.ks@zegrapher.com>
**
**  This file is part of ZeCalculator's source code.
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

#include <zecalculator/math_objects/forward_declares.h>

namespace zc {

template <parsing::Type type>
using CppUnaryFunction = CppFunction<type, 1>;

template <parsing::Type type>
using CppBinaryFunction = CppFunction<type, 2>;

namespace ast {

  template <size_t args_num>
  using Function = zc::Function<parsing::Type::AST, args_num>;

  using Sequence = zc::Sequence<parsing::Type::AST>;
  using Expression = zc::Expression<parsing::Type::AST>;
  using GlobalVariable = zc::GlobalVariable<parsing::Type::AST>;

  template <size_t args_num>
  using CppFunction = zc::CppFunction<parsing::Type::AST, args_num>;

  using CppUnaryFunction = CppFunction<1>;
  using CppBinaryFunction = CppFunction<2>;

  using GlobalConstant = zc::GlobalConstant<parsing::Type::AST>;

  using Unkown = zc::Unknown<parsing::Type::AST>;

  using DynMathObject = zc::DynMathObject<parsing::Type::AST>;

} // namespace ast

namespace rpn {

  template <size_t args_num>
  using Function = zc::Function<parsing::Type::RPN, args_num>;

  using Sequence = zc::Sequence<parsing::Type::RPN>;
  using Expression = zc::Expression<parsing::Type::RPN>;
  using GlobalVariable = zc::GlobalVariable<parsing::Type::RPN>;

  template <size_t args_num>
  using CppFunction = zc::CppFunction<parsing::Type::RPN, args_num>;

  using CppUnaryFunction = CppFunction<1>;
  using CppBinaryFunction = CppFunction<2>;

  using GlobalConstant = zc::GlobalConstant<parsing::Type::RPN>;

  using Unkown = zc::Unknown<parsing::Type::RPN>;

  using DynMathObject = zc::DynMathObject<parsing::Type::RPN>;

} // namespace rpn

}
