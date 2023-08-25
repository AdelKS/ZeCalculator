/****************************************************************************
**  Copyright (c) 2023, Adel Kara Slimane <adel.ks@zegrapher.com>
**
**  This file is part of ZeGrapher's source code.
**
**  ZeGrapher is free software: you may copy, redistribute and/or modify it
**  under the terms of the GNU General Public License as published by the
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

Expression::Expression(const std::string& expr)
{
  set_expression(expr);
}

tl::expected<double, eval::Error> Expression::evaluate(const MathWorld& world) const
{
  return Function::evaluate({}, world);
}

tl::expected<double, eval::Error> Expression::operator ()(const MathWorld& world) const
{
  return Function::evaluate({}, world);
}

tl::expected<double, eval::Error> Expression::evaluate(const MathWorld& world, size_t current_recursion_depth) const
{
  return Function::evaluate({}, world, current_recursion_depth);
}

}
