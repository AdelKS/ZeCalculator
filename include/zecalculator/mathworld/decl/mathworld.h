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

#include "zecalculator/parsing/parser.h"
#include <zecalculator/math_objects/builtin_binary_functions.h>
#include <zecalculator/math_objects/builtin_unary_functions.h>
#include <zecalculator/mathworld/mathworld_template.h>

namespace zc {

template <parsing::Type>
class Function;

template <parsing::Type>
class Sequence;

struct GlobalConstant;

template <parsing::Type>
class Expression;

template <parsing::Type type>
using GlobalVariable = Expression<type>;

template <parsing::Type type>
using MathWorld = MathWorldT<CppUnaryFunction,
                             CppBinaryFunction,
                             GlobalConstant,
                             Function<type>,
                             GlobalVariable<type>,
                             Sequence<type>>;

namespace ast {

using MathWorld = MathWorldT<CppUnaryFunction,
                             CppBinaryFunction,
                             GlobalConstant,
                             Function<parsing::AST>,
                             GlobalVariable<parsing::AST>,
                             Sequence<parsing::AST>>;

}

namespace rpn {

using MathWorld = MathWorldT<CppUnaryFunction,
                             CppBinaryFunction,
                             GlobalConstant,
                             Function<parsing::RPN>,
                             GlobalVariable<parsing::RPN>,
                             Sequence<parsing::RPN>>;

}

}
