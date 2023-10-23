#pragma once

#include <cstddef>
#include <zecalculator/parsing/shared.h>

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

namespace zc {

template <parsing::Type, size_t>
class Function;

template <parsing::Type>
class Sequence;

/// @brief a class that represents a general expression
/// @note  an expression is a function that does not have any input
template <parsing::Type type>
using Expression = Function<type, 0>;

/// @brief there's not mathematical difference between a global variable
///        and a simple mathematical expression. It just makes more sense
///        when we add one to a math world: a global variable is an expression
///        that has a name
template <parsing::Type type>
using GlobalVariable = Function<type, 0>;

template <parsing::Type type>
class GlobalConstant;

template <parsing::Type, size_t args_num>
  requires (args_num > 0)
class CppFunction;

}
