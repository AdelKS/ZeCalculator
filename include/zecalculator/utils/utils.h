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

#include <type_traits>
#include <utility>

namespace zc {
namespace utils {

  // useful for visiting std::variant
  // taken from https://en.cppreference.com/w/cpp/utility/variant/visit
  template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
  template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

  template <class T, class... U>
  concept is_any_of = (std::is_same_v<T, U> or ...);

  template <class T, class... U>
  concept is_none_of = ((not std::is_same_v<T, U>) and ...);

  template <int>
  inline constexpr bool dependent_false_num_v = false;

  template <class Fn, class Int, Int... v>
  constexpr void for_int_seq(Fn&& f, std::integer_sequence<Int, v...>)
  {
    (f(std::integral_constant<Int, v>()), ...);
  }

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
} // namespace zc
