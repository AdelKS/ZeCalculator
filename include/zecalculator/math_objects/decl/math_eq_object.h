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

#include <string>

#include <zecalculator/math_objects/math_object.h>
#include <zecalculator/parsing/data_structures/decl/ast.h>
#include <zecalculator/parsing/data_structures/deps.h>
#include <zecalculator/parsing/types.h>

namespace zc {

template <parsing::Type type>
class MathWorld;

template <parsing::Type type>
class Unknown;

template <parsing::Type type>
class MathEqObject: public MathObject<type>
{
public:
  const std::string& equation() const { return m_equation; };

  /// @brief gives the Functions and Variables this object directly depends on
  /// @note  uses only the function's expression (no name lookup is done in the MathWorld)
  /// @note  undefined functions & variables in the math world will still be listed
  std::unordered_map<std::string, deps::ObjectType> direct_dependencies() const;

protected:
  MathEqObject(MathObject<type> math_obj, std::string equation);

  MathEqObject(const MathEqObject&) = default;
  MathEqObject& operator = (const MathEqObject&) = default;

  /// @brief full function definition
  /// @note needs to be in the format "[function signature] = [expression]" or "[variable name] = [expression]"
  /// @example "f(x) = cos(x)"
  /// @example "volume = 2.0"
  std::string m_equation;

  /// @brief the left side to the equal sign in the definition
  parsing::AST lhs;

  /// @brief the right side to the equal sign in the definition
  parsing::AST rhs;

  template <parsing::Type>
  friend class MathWorld;

};

}
