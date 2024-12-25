#pragma once

#include <cstddef>
#include <zecalculator/parsing/types.h>

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

template <parsing::Type>
class Function;

template <parsing::Type>
class Sequence;

template <parsing::Type>
class Data;

struct GlobalConstant;

template <size_t args_num>
  requires (args_num > 0)
struct CppFunction;

template <parsing::Type type>
class Unknown;

template <parsing::Type type>
class DynMathObject;

}
