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

#include <zecalculator/utils/tuple.h>
#include <zecalculator/math_objects/aliases.h>
#include <zecalculator/parsing/types.h>

#include <tuple>

namespace zc {

/// @brief contains all the math objects that are defined through an equation
template <parsing::Type type>
using MathEqObjects = std::tuple<GlobalConstant,
                                 Function<type>,
                                 Sequence<type>>;

template <parsing::Type type>
using MathObjects = tuple_type_cat_t<MathEqObjects<type>, std::tuple<CppFunction<1>, CppFunction<2>>>;

template <parsing::Type type>
using MathObjectsVariant = to_variant_t<MathObjects<type>>;

/// @brief maximum number of arguments functions can take
/// @todo template the rest of the code to take this as input
constexpr size_t max_func_args = 2;

}
