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

template <class T>
class CppFunctionImpl;

// trick taken from https://stackoverflow.com/questions/48818462/is-there-any-way-for-a-c-template-function-to-take-exactly-n-arguments
template <size_t i, class T>
using alwaysT = T;

template <size_t... i>
  requires (sizeof...(i) > 0)
class CppFunctionImpl<std::integer_sequence<size_t, i...>>
{
public:
  using CppFunctionPtr = double (*) (alwaysT<i, double>...);
  static constexpr size_t args_num = sizeof...(i);

  constexpr CppFunctionImpl() = default;

  constexpr CppFunctionImpl(CppFunctionPtr f_ptr) : f_ptr(f_ptr) {};

  void set_name(std::string name) { this->name = std::move(name); }

  const std::string& get_name() const { return name; }

  constexpr void set(CppFunctionImpl f) {f_ptr = f.f_ptr; }

  double operator()(alwaysT<i, double>... val) const
  {
    return f_ptr(val...);
  }

  bool operator == (const CppFunctionImpl&) const = default;

protected:
  CppFunctionPtr f_ptr = nullptr;
  std::string name;
};

template <size_t args_num>
  requires (args_num > 0)
struct CppFunction: CppFunctionImpl<std::make_integer_sequence<size_t, args_num>>
{
  using Parent = CppFunctionImpl<std::make_integer_sequence<size_t, args_num>>;
  using Parent::Parent;

};

} // namespace zc
