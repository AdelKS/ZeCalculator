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

#include <zecalculator/math_objects/decl/math_eq_object.h>
#include <zecalculator/parsing/data_structures/token.h>
#include <zecalculator/error.h>

namespace zc {

template <parsing::Type type>
class MathWorld;

template <parsing::Type type>
class GlobalConstant: public MathEqObject<type>
{
public:

  /// @brief changes the value stored along with the equation with the new value
  GlobalConstant& set(double val);

  /// @brief changes the value stored, leaves the equation unchanged and outdated
  /// @note only used for benchmarks to avoid measuring the string operations
  GlobalConstant& set_fast(double val);

  GlobalConstant& operator = (double val);

  bool operator == (double val) const;

  /// @brief perform operation if there's a value, do nothing otherwise
  GlobalConstant& operator += (double val);
  GlobalConstant& operator -= (double val);
  GlobalConstant& operator /= (double val);
  GlobalConstant& operator *= (double val);

  double value() const;

protected:
  GlobalConstant(MathEqObject<type> math_expr_obj, parsing::tokens::Number m_value);

  parsing::tokens::Number m_value;

  template <parsing::Type>
  friend class MathWorld;
};

}
