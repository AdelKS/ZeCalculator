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

using CppUnaryFunction = CppFunction<1>;
using CppBinaryFunction = CppFunction<2>;

namespace fast {

  using Function = zc::Function<parsing::Type::FAST>;
  using Sequence = zc::Sequence<parsing::Type::FAST>;

  using CppUnaryFunction = CppFunction<1>;
  using CppBinaryFunction = CppFunction<2>;

  using GlobalConstant = zc::GlobalConstant<parsing::Type::FAST>;

  using Unkown = zc::Unknown<parsing::Type::FAST>;

  using DynMathObject = zc::DynMathObject<parsing::Type::FAST>;

} // namespace fast

namespace rpn {

  using Function = zc::Function<parsing::Type::RPN>;
  using Sequence = zc::Sequence<parsing::Type::RPN>;

  using GlobalConstant = zc::GlobalConstant<parsing::Type::RPN>;

  using Unkown = zc::Unknown<parsing::Type::RPN>;

  using DynMathObject = zc::DynMathObject<parsing::Type::RPN>;

} // namespace rpn

}
