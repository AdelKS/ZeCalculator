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

#include <cstddef>
#include <utility>
#include <string>

#include <zecalculator/parsing/types.h>
#include <zecalculator/utils/utils.h>
#include <zecalculator/math_objects/decl/math_object.h>

namespace zc {

template <parsing::Type type>
class MathWorld;

/// @brief function signature of the type double (*) (double, double, ...) [args_num doubles as input]
template <size_t args_num>
using CppMathFunctionPtr = typename utils::math_func_signature_t<args_num>;

template <parsing::Type type, size_t args_num>
  requires (args_num > 0)
class CppFunction: public MathObject<type>
{
public:

  constexpr void set(CppMathFunctionPtr<args_num> ptr);

  template <class... DBL>
    requires((std::is_convertible_v<DBL, double> and ...) and sizeof...(DBL) == args_num)
  double operator()(DBL... val) const;

  bool operator == (const CppFunction&) const = default;

protected:

  constexpr CppFunction(size_t slot, MathWorld<type>* mathworld);

  CppMathFunctionPtr<args_num> f_ptr = nullptr;

  template <parsing::Type>
  friend class MathWorld;
};

} // namespace zc
