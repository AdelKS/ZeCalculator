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

namespace zc {

template <size_t args_num>
  requires(args_num > 0)
double CppFunction<args_num>::operator()(std::array<double, args_num> vals) const
{
  auto unpack_compute = [&]<size_t... i>(std::integer_sequence<size_t, i...>)
  {
    return f_ptr(vals[i]...);
  };
  return unpack_compute(std::make_index_sequence<args_num>());
}

template <size_t args_num>
  requires(args_num > 0)
double CppFunction<args_num>::operator()(std::span<const double, args_num> vals) const
{
  auto unpack_compute = [&]<size_t... i>(std::integer_sequence<size_t, i...>)
  {
    return f_ptr(vals[i]...);
  };
  return unpack_compute(std::make_index_sequence<args_num>());
}

} // namespace zc
