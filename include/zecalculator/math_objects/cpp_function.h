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

namespace zc {

namespace internal {

  // trick taken from https://stackoverflow.com/questions/48818462/is-there-any-way-for-a-c-template-function-to-take-exactly-n-arguments
  template <size_t i, class T>
  using alwaysT = T;

  template <class T>
  struct math_func_signature;

  template <size_t... i>
    requires (sizeof...(i) > 0)
  struct math_func_signature<std::index_sequence<i...>>
  {
    using type = double (*) (alwaysT<i, double>...);
  };

  template <size_t args_num>
  using math_func_signature_t = typename math_func_signature<std::make_index_sequence<args_num>>::type;
} // namespace internal

/// @brief function signature of the type double (*) (double, double, ...) [args_num doubles as input]
template <size_t args_num>
using CppMathFunctionPtr = typename internal::math_func_signature_t<args_num>;

template <size_t args_num>
  requires (args_num > 0)
struct CppFunction
{
public:

  constexpr CppFunction() = default;

  constexpr CppFunction(CppMathFunctionPtr<args_num> f_ptr) : f_ptr(f_ptr) {};

  void set_name(std::string name) { this->name = std::move(name); }

  const std::string& get_name() const { return name; }

  constexpr void set(const CppFunction& f) {f_ptr = f.f_ptr; }

  template <class... DBL>
    requires((std::is_convertible_v<DBL, double> and ...) and sizeof...(DBL) == args_num)
  double operator()(DBL... val) const
  {
    return f_ptr(val...);
  }

  bool operator == (const CppFunction&) const = default;

protected:
  CppMathFunctionPtr<args_num> f_ptr = nullptr;
  std::string name;
};

} // namespace zc
