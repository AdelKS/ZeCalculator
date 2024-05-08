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

#include <zecalculator/evaluation/impl/evaluation.h>
#include <zecalculator/math_objects/decl/function.h>
#include <zecalculator/parsing/data_structures/impl/ast.h>
#include <zecalculator/parsing/data_structures/impl/shared.h>
#include <zecalculator/parsing/impl/parser.h>

namespace zc {

template <parsing::Type type>
Function<type>::Function(MathObject base, size_t argument_number)
  : MathObject(std::move(base)), argument_number(argument_number)
{}

template <parsing::Type type>
tl::expected<double, Error> Function<type>::evaluate(
  std::span<const double> args, size_t current_recursion_depth) const
{
  if (this->args_num() != args.size()) [[unlikely]]
    return tl::unexpected(Error::cpp_incorrect_argnum());

  // 'bound_rhs' should always have a value
  // except  in the brief moment where MathWorld is rebinding
  // function objects in 'rebind_functions()'
  assert(bound_rhs);

  return zc::evaluate(*bound_rhs, args, current_recursion_depth);
}

template <parsing::Type type>
tl::expected<double, Error> Function<type>::evaluate(std::span<const double> args) const
{
  // this function is user called, so the recursion depth is zero
  return evaluate(args, 0);
}

template <parsing::Type type>
tl::expected<double, Error> Function<type>::operator()(std::span<const double> args) const
{
  return evaluate(args, 0);
}

template <parsing::Type type>
template <class... DBL>
  requires (std::is_convertible_v<DBL, double> and ...)
tl::expected<double, Error> Function<type>::evaluate(DBL... val) const
{
  return evaluate(std::array<double, sizeof...(DBL)>{double(val)...}, 0);
}

template <parsing::Type type>
template <class... DBL>
  requires (std::is_convertible_v<DBL, double> and ...)
tl::expected<double, Error> Function<type>::operator()(DBL... val) const
{
  return evaluate(std::array<double, sizeof...(DBL)>{double(val)...}, 0);
}

} // namespace zc
