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

#include <string_view>

#include <zecalculator/math_objects/cpp_function.h>

namespace zc {

using CppUnaryFunction = CppFunction<1>;

// we save the names along with the function pointers for convenience
// we could save only the function pointers, and the names only in the inventory
inline constexpr std::array<std::pair<std::string_view, CppMathFunctionPtr<1>>, 30> builtin_unary_functions = {{
  {"cos",   CppMathFunctionPtr<1>(std::cos)},
  {"sin",   CppMathFunctionPtr<1>(std::sin)},
  {"tan",   CppMathFunctionPtr<1>(std::tan)},

  {"acos",  CppMathFunctionPtr<1>(std::acos)},
  {"asin",  CppMathFunctionPtr<1>(std::asin)},
  {"atan",  CppMathFunctionPtr<1>(std::atan)},

  {"cosh",  CppMathFunctionPtr<1>(std::cosh)},
  {"sinh",  CppMathFunctionPtr<1>(std::sinh)},
  {"tanh",  CppMathFunctionPtr<1>(std::tanh)},

  {"ch",    CppMathFunctionPtr<1>(std::cosh)},
  {"sh",    CppMathFunctionPtr<1>(std::sinh)},
  {"th",    CppMathFunctionPtr<1>(std::tanh)},

  {"acosh", CppMathFunctionPtr<1>(std::acosh)},
  {"asinh", CppMathFunctionPtr<1>(std::asinh)},
  {"atanh", CppMathFunctionPtr<1>(std::atanh)},

  {"ach",   CppMathFunctionPtr<1>(std::acosh)},
  {"ash",   CppMathFunctionPtr<1>(std::asinh)},
  {"ath",   CppMathFunctionPtr<1>(std::atanh)},

  {"sqrt",  CppMathFunctionPtr<1>(std::sqrt)},
  {"log",   CppMathFunctionPtr<1>(std::log10)},
  {"lg",    CppMathFunctionPtr<1>(std::log2)},
  {"ln",    CppMathFunctionPtr<1>(std::log)},
  {"abs",   CppMathFunctionPtr<1>(std::abs)},
  {"exp",   CppMathFunctionPtr<1>(std::exp)},
  {"floor", CppMathFunctionPtr<1>(std::floor)},
  {"ceil",  CppMathFunctionPtr<1>(std::ceil)},
  {"erf",   CppMathFunctionPtr<1>(std::erf)},
  {"erfc",  CppMathFunctionPtr<1>(std::erfc)},
  {"gamma", CppMathFunctionPtr<1>(std::tgamma)},
  {"Γ",     CppMathFunctionPtr<1>(std::tgamma)},
}};

inline CppMathFunctionPtr<1> unary_func_from_name(std::string_view name)
{
  for(auto&& [f_name, f]: builtin_unary_functions)
  {
    if (f_name == name)
      return f;
  };
  return nullptr;
}

}
