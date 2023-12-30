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
#include <zecalculator/error.h>

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

  /// @brief perform operation if there's a value, do nothing otherwise
  GlobalConstant& operator += (double val);
  GlobalConstant& operator -= (double val);
  GlobalConstant& operator /= (double val);
  GlobalConstant& operator *= (double val);

  operator bool () const;

  operator tl::expected<double, Error> () const;

  const double& operator * () const;
  double& operator * ();

  const double& value() const;
  double& value();

  const Error& error() const;

protected:
  GlobalConstant(size_t slot, class MathWorld<type>* mathworld);

  tl::expected<double, Error> exp_value = tl::unexpected(Error::empty_expression());

  template <parsing::Type>
  friend class MathWorld;
};

}
