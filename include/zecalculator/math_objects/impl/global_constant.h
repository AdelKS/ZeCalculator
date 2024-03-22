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

#include <zecalculator/math_objects/decl/global_constant.h>

namespace zc {

template <parsing::Type type>
GlobalConstant<type>::GlobalConstant(MathWorldObjectHandle<type> obj_handle)
  : MathObject<type>(obj_handle)
{}

template <parsing::Type type>
void GlobalConstant<type>::set(double val)
{
  *this = val;
}

template <parsing::Type type>
bool GlobalConstant<type>::operator == (double val) const
{
  if (exp_value)
    return *exp_value == val;
  else return false;
}

template <parsing::Type type>
GlobalConstant<type>& GlobalConstant<type>::operator = (double val)
{
  exp_value = val;
  return *this;
}

template <parsing::Type type>
GlobalConstant<type>& GlobalConstant<type>::operator += (double val)
{
  if (*this)
    **this += val;
  return *this;
}

template <parsing::Type type>
GlobalConstant<type>& GlobalConstant<type>::operator -= (double val)
{
  if (*this)
    **this -= val;
  return *this;
}

template <parsing::Type type>
GlobalConstant<type>& GlobalConstant<type>::operator *= (double val)
{
  if (*this)
    **this *= val;
  return *this;
}

template <parsing::Type type>
GlobalConstant<type>& GlobalConstant<type>::operator /= (double val)
{
  if (*this)
    **this /= val;
  return *this;
}

template <parsing::Type type>
GlobalConstant<type>::operator bool () const
{
  return bool(exp_value);
}

template <parsing::Type type>
GlobalConstant<type>::operator tl::expected<double, Error> () const
{
  return exp_value;
}

template <parsing::Type type>
const double& GlobalConstant<type>::value() const
{
  return exp_value.value();
}

template <parsing::Type type>
double& GlobalConstant<type>::value()
{
  return exp_value.value();
}

template <parsing::Type type>
double& GlobalConstant<type>::operator * ()
{
  return *exp_value;
}

template <parsing::Type type>
const double& GlobalConstant<type>::operator * () const
{
  return *exp_value;
}


template <parsing::Type type>
const Error& GlobalConstant<type>::error() const
{
  return exp_value.error();
}

}
