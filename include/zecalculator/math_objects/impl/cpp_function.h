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

#include <zecalculator/math_objects/decl/cpp_function.h>
#include <zecalculator/math_objects/impl/math_object.h>

namespace zc {

template <parsing::Type type, size_t args_num>
  requires(args_num > 0)
constexpr void CppFunction<type, args_num>::set(CppMathFunctionPtr<args_num> ptr)
{
  f_ptr = ptr;
}

template <parsing::Type type, size_t args_num>
  requires(args_num > 0)
template <class... DBL>
  requires((std::is_convertible_v<DBL, double> and ...) and sizeof...(DBL) == args_num)
double CppFunction<type, args_num>::operator()(DBL... val) const
{
  return f_ptr(val...);
}

template <parsing::Type type, size_t args_num>
  requires(args_num > 0)
constexpr CppFunction<type, args_num>::CppFunction(MathWorldObjectHandle<type> obj_handle)
  : MathObject<type>(obj_handle)
{}

template <parsing::Type type, size_t args_num>
  requires(args_num > 0)
constexpr CppFunction<type, args_num>::CppFunction(MathObject<type> obj, CppMathFunctionPtr<args_num> ptr)
  : MathObject<type>(std::move(obj)), f_ptr(ptr)
{}

} // namespace zc
