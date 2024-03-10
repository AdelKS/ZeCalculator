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

#include <zecalculator/math_objects/decl/dyn_math_object.h>
#include <zecalculator/math_objects/impl/cpp_function.h>
#include <zecalculator/math_objects/impl/function.h>
#include <zecalculator/math_objects/impl/global_constant.h>
#include <zecalculator/math_objects/impl/sequence.h>
#include <zecalculator/utils/utils.h>

namespace zc {

template <parsing::Type type>
template <class... DBL>
  requires (std::is_convertible_v<DBL, double> and ...)
tl::expected<double, Error> DynMathObject<type>::evaluate(DBL... val) const
{
  using Ret = tl::expected<double, Error>;
  return std::visit(
    utils::overloaded{
      [&]<size_t args_num>(const CppFunction<type, args_num>& cpp_f) -> Ret
      {
        if constexpr (sizeof...(val) != args_num)
          return tl::unexpected(Error::cpp_incorrect_argnum());
        else return cpp_f(val...);
      },
      [&]<size_t args_num>(const Function<type, args_num>& f) -> Ret
      {
        if constexpr (sizeof...(val) != args_num)
          return tl::unexpected(Error::cpp_incorrect_argnum());
        else if constexpr (sizeof...(val) != 0)
          return f(std::array{double(val)...});
        else return f();
      },
      [&](const GlobalConstant<type>& cst) -> Ret
      {
        if constexpr (sizeof...(val) != 0)
          return tl::unexpected(Error::cpp_incorrect_argnum());
        else return cst;
      },
      [&](const Sequence<type>& u) -> Ret
      {
        if constexpr (sizeof...(val) != 1)
          return tl::unexpected(Error::cpp_incorrect_argnum());
        else return u(val...);
      }
    },
    variant
  );
}

template <parsing::Type type>
template <class... DBL>
  requires (std::is_convertible_v<DBL, double> and ...)
tl::expected<double, Error> DynMathObject<type>::operator () (DBL... val) const
{
  return evaluate(val...);
}

}
