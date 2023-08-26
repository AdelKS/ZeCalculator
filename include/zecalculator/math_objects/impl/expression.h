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

#pragma once

#include <zecalculator/math_objects/decl/expression.h>

#include <zecalculator/math_objects/impl/function.h>

/* TODO: update approach as the following:
   - Check for validity
   - Enable setting custom names for functions and variables
   - Performance improvement: flatten trees
*/

namespace zc {

namespace eval {
  struct Variable;
}

template <parsing::Type type>
Expression<type>::Expression(const std::string& expr)
{
  this->set_expression(expr);
}

template <parsing::Type type>
tl::expected<double, eval::Error> Expression<type>::evaluate(const MathWorld<type>& world) const
{
  return Function<type>::evaluate({}, world);
}

template <parsing::Type type>
tl::expected<double, eval::Error> Expression<type>::operator ()(const MathWorld<type>& world) const
{
  return Function<type>::evaluate({}, world);
}

template <parsing::Type type>
tl::expected<double, eval::Error> Expression<type>::evaluate(const MathWorld<type>& world, size_t current_recursion_depth) const
{
  return Function<type>::evaluate({}, world, current_recursion_depth);
}

}
