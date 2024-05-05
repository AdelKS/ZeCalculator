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
GlobalConstant<type>::GlobalConstant(MathEqObject<type> math_expr_obj, parsing::Token m_value)
  : MathEqObject<type>(std::move(math_expr_obj)), m_value(std::move(m_value))
{}

template <parsing::Type type>
GlobalConstant<type>& GlobalConstant<type>::set(double val)
{
  std::string val_str = std::to_string(val);
  this->m_equation.replace(m_value.begin, m_value.substr.size(), val_str);

  m_value.substr = val_str;
  m_value.value = val;

  return *this;
}

template <parsing::Type type>
GlobalConstant<type>& GlobalConstant<type>::set_fast(double val)
{
  m_value.value = val;

  return *this;
}

template <parsing::Type type>
bool GlobalConstant<type>::operator == (double val) const
{
  return m_value.value == val;
}

template <parsing::Type type>
GlobalConstant<type>& GlobalConstant<type>::operator = (double val)
{
  set(val);
  return *this;
}

template <parsing::Type type>
GlobalConstant<type>& GlobalConstant<type>::operator += (double val)
{
  set(m_value.value + val);
  return *this;
}

template <parsing::Type type>
GlobalConstant<type>& GlobalConstant<type>::operator -= (double val)
{
  set(m_value.value - val);
  return *this;
}

template <parsing::Type type>
GlobalConstant<type>& GlobalConstant<type>::operator *= (double val)
{
  set(m_value.value * val);
  return *this;
}

template <parsing::Type type>
GlobalConstant<type>& GlobalConstant<type>::operator /= (double val)
{
  set(m_value.value / val);
  return *this;
}

template <parsing::Type type>
double GlobalConstant<type>::value() const
{
  return m_value.value;
}

}
