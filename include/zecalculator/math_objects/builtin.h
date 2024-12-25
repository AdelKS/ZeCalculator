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

#include <zecalculator/math_objects/cpp_function.h>
#include <zecalculator/math_objects/global_constant.h>

namespace zc {

inline const std::array builtin_global_constants = std::to_array<GlobalConstant>(
{
  {"math::pi",    3.141592653589793},
  {"math::π",     3.141592653589793},
  {"physics::kB", 1.380649e-23},   // Blotzmann constant, SI units
  {"physics::h",  6.62607015e-34}, // Plank constant, SI units
  {"physics::c",  299792458},      // Speed of light in vacuum, SI units
});

// we save the names along with the function pointers for convenience
// we could save only the function pointers, and the names only in the inventory
inline const std::array builtin_unary_functions = std::to_array<CppFunction<1>>({
  {"cos", std::cos},     {"sin", std::sin},      {"tan", std::tan},
  {"acos", std::acos},   {"asin", std::asin},    {"atan", std::atan},
  {"cosh", std::cosh},   {"sinh", std::sinh},    {"tanh", std::tanh},
  {"ch", std::cosh},     {"sh", std::sinh},      {"th", std::tanh},
  {"acosh", std::acosh}, {"asinh", std::asinh},  {"atanh", std::atanh},
  {"ach", std::acosh},   {"ash", std::asinh},    {"ath", std::atanh},
  {"sqrt", std::sqrt},   {"log", std::log10},    {"lg", std::log2},
  {"ln", std::log},      {"abs", std::abs},      {"exp", std::exp},
  {"floor", std::floor}, {"ceil", std::ceil},    {"erf", std::erf},
  {"erfc", std::erfc},   {"gamma", std::tgamma}, {"Γ", std::tgamma},
});
}
