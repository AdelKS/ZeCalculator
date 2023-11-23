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
#include <memory>

namespace zc {

/// @brief non_unique_ptr but with copies
template <class T>
struct non_unique_ptr: std::unique_ptr<T>
{
  using Parent = std::unique_ptr<T>;

  non_unique_ptr(T val) : Parent(std::make_unique<T>(std::move(val)))
  {}

  template <class U>
    requires (utils::is_none_of<std::remove_cvref_t<U>, Parent, non_unique_ptr>)
  non_unique_ptr(U&& val) : Parent(std::make_unique<T>(std::forward<U>(val)))
  {}

  non_unique_ptr(Parent&& u_ptr)
    : Parent(std::move(u_ptr))
  {}

  non_unique_ptr(const Parent& u_ptr)
    : Parent(u_ptr ? std::make_unique<T>(*u_ptr) : nullptr)
  {}

  non_unique_ptr(const non_unique_ptr& other)
    : Parent(other ? std::make_unique<T>(*other) : nullptr)
  {}

  non_unique_ptr& operator = (const non_unique_ptr& other)
  {
    reset(other ? std::make_unique<T>(*other) : nullptr);
  }

  non_unique_ptr(non_unique_ptr&&) = default;
  non_unique_ptr& operator = (non_unique_ptr&&) = default;

  bool operator == (const non_unique_ptr& other) const
    requires (std::equality_comparable<T>)
  {
    if (this->get() == other.get())
      return true;
    else if (not *this or not other)
      return false;
    else return **this == *other;
  }
};

}
