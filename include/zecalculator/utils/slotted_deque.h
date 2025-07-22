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
#include <deque>
#include <optional>
#include <vector>

namespace zc {

/// @brief a vector of T, where each element keeps its index
///        during its whole lifetime
template <class T>
class SlottedDeque
{

public:
  SlottedDeque() = default;

  size_t size() const
  {
    return opt_vals.size();
  }

  /// @brief returns the next free slot
  size_t next_free_slot() const
  {
    if (free_slots.empty())
      return size();
    else return free_slots.back();
  }

  /// @brief finds the a free slot, puts 'val' in it, then returns the slot index
  size_t push(T val)
  {
    if (free_slots.empty())
    {
      const size_t slot = size();
      opt_vals.push_back(std::move(val));
      return slot;
    }
    else
    {
      const size_t slot = free_slots.back();
      free_slots.pop_back();
      opt_vals[slot].emplace(std::move(val));
      return slot;
    }
  }

  void push(T val, size_t slot)
  {
    if (size() <= slot)
    {
      const size_t old_size = size();
      opt_vals.resize(slot+1);

      free_slots.reserve(free_slots.size() + (slot - old_size + 1));
      for (size_t i = slot - 1 ; i != old_size - 1 ; i--)
        free_slots.push_back(i);
    }

    if (auto it = std::ranges::find(free_slots, slot); it != free_slots.end())
      free_slots.erase(it);

    opt_vals[slot].emplace(std::move(val));
  }

  template <class... U>
  size_t emplace(U&&... args)
  {
    if (free_slots.empty())
    {
      const size_t slot = size();
      opt_vals.emplace_back(std::in_place_t{}, std::forward<U>(args)...);
      return slot;
    }
    else
    {
      const size_t slot = free_slots.back();
      free_slots.pop_back();
      opt_vals[slot].emplace(std::forward<U>(args)...);
      return slot;
    }
  }

  ///@brief frees the slot 'slot'
  void free(size_t slot)
  {
    if (slot < size())
    {
      opt_vals[slot].reset();
      if (std::ranges::find(free_slots, slot) == free_slots.end())
        free_slots.push_back(slot);
    }
  }

  /// @brief returns if slot is taken and assigned
  bool is_assigned(size_t slot) const
  {
    return slot < size() and opt_vals[slot];
  }

  /// @brief returns the element T at 'slot', bounds checked
  const T& at(size_t slot) const
  {
    if (not is_assigned(slot))
      throw std::range_error("Accessing unassigned slot");

    return *opt_vals[slot];
  }

  /// @brief returns the element T at 'slot', bounds checked
  T& at(size_t slot)
  {
    if (not is_assigned(slot))
      throw std::range_error("Accessing unassigned slot");

    return *opt_vals[slot];
  }

  /// @brief returns the element T at 'slot'
  const T& operator [] (size_t slot) const
  {
    return *opt_vals[slot];
  }

  /// @brief returns the element T at 'slot'
  T& operator [] (size_t slot)
  {
    return *opt_vals[slot];
  }

  /// @brief clears the container
  void clear()
  {
    opt_vals.clear();
  }

  // iterator stuff

  template <bool is_const>
  struct iter
  {
    using difference_type = std::ptrdiff_t;
    using value_type = std::conditional_t<is_const, const T, T>;

    /// @brief moves to the next assigned value within the SlottedDeque
    iter& operator++()
    {
      current_slot++;
      while (current_slot != container->size() and not container->is_assigned(current_slot))
        current_slot++;
      return *this;
    }

    iter operator ++ (int)
    {
      iter temp = *this;
      ++*this;
      return temp;
    }

    value_type& operator*() const
    {
      assert(container->is_assigned(current_slot));
      return (*container)[current_slot];
    }

    value_type* operator->() const
    {
      assert(container->is_assigned(current_slot));
      return &((*container)[current_slot]);
    }

    bool operator == (const iter& other) const
    {
      assert(container == other.container);
      return current_slot == other.current_slot;
    }

    iter() = default;

    iter(const iter&) = default;
    iter(iter&&) = default;

    iter& operator = (const iter&) = default;
    iter& operator = (iter&&) = default;

  protected:
    using DequeType = std::conditional_t<is_const, const SlottedDeque<T>*, SlottedDeque<T>*>;

    iter(size_t current_slot, DequeType container) : current_slot(current_slot), container(container) {}

    size_t current_slot = 0;
    DequeType container = nullptr;

    friend class SlottedDeque<T>;
  };

  using iterator = iter<false>;
  using const_iterator = iter<true>;

  iterator begin()
  {
    return iterator(first_assigned_slot(), this);
  }

  const_iterator begin() const
  {
    return const_iterator(first_assigned_slot(), this);
  }

  const_iterator cbegin() const
  {
    return const_iterator(first_assigned_slot(), this);
  }

  iterator end()
  {
    return iterator(size(), this);
  }

  const_iterator end() const
  {
    return const_iterator(size(), this);
  }

  const_iterator cend() const
  {
    return const_iterator(size(), this);
  }

  template <bool c>
  void free(iter<c> it)
  {
    free(it.current_slot);
  }

protected:

  /// @brief returns the first assigned slot, or the size of the container if there's none
  size_t first_assigned_slot() const
  {
    size_t current_slot = 0;
    while (current_slot != size() and not is_assigned(current_slot))
      current_slot++;

    return current_slot;
  }

  std::vector<size_t> free_slots;
  std::deque<std::optional<T>> opt_vals;
};

}
