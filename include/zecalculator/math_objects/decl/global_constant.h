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

#include <zecalculator/math_objects/decl/math_object.h>

namespace zc {

template <parsing::Type type>
class MathWorld;

template <parsing::Type type>
class GlobalConstant: public MathObject<type>
{
public:

  void set(double val);

  GlobalConstant& operator = (double val);

  bool operator == (double val) const;

  double value = 0;

  /// @brief perform operation if there's a value
  GlobalConstant& operator += (double val);
  GlobalConstant& operator -= (double val);
  GlobalConstant& operator /= (double val);
  GlobalConstant& operator *= (double val);

protected:
  GlobalConstant(size_t slot, class MathWorld<type>* mathworld);

  template <parsing::Type>
  friend class MathWorld;
};

}
