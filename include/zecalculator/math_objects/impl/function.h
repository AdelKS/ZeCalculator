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
Function<type>::Function(MathObject base, std::string equation, size_t argument_number)
  : MathObject(std::move(base)), equation(std::move(equation)), argument_number(argument_number)
{}

template <parsing::Type type>
Function<type>::Function(MathObject base, std::string equation, size_t argument_number, parsing::Parsing<type> parsing)
  : MathObject(std::move(base)), bound_rhs(std::move(parsing)), equation(std::move(equation)), argument_number(argument_number)
{}

template <parsing::Type type>
tl::expected<double, Error> Function<type>::evaluate(std::initializer_list<double> args) const
{
  assert(bound_rhs);
  return zc::evaluate(*bound_rhs, std::span(args.begin(), args.size()));
}

template <parsing::Type type>
tl::expected<double, Error> Function<type>::operator()(std::initializer_list<double> args) const
{
  return evaluate(args);
}

} // namespace zc
