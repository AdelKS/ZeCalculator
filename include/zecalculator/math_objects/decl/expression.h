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

#include <zecalculator/math_objects/decl/function.h>

/* TODO: update approach as the following:
   - Check for validity
   - Enable setting custom names for functions and variables
   - Performance improvement: flatten trees
*/

namespace zc {

namespace eval {
  struct Variable;
}

/// @brief a class that represents a general expression
/// @note  an expression is a function that does not have any input
class Expression: public ast::Function
{
public:
  explicit Expression() = default;

  explicit Expression(const std::string& expr);

  tl::expected<double, eval::Error> evaluate(const MathWorld& world) const;

  tl::expected<double, eval::Error> operator ()(const MathWorld& world) const;

protected:
  tl::expected<double, eval::Error> evaluate(const MathWorld& world, size_t current_recursion_depth) const;

  friend tl::expected<double, eval::Error> evaluate(const ast::Tree& tree,
                                                    const name_map<double>& input_vars,
                                                    const MathWorld& world,
                                                    size_t current_recursion_depth);

  friend struct eval::Variable;

  // hide functions that are not needed from Function
  using Function::evaluate;
  using Function::set_input_vars;

};

}
