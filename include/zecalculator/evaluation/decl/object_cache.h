#pragma once

/****************************************************************************
**  Copyright (c) 2024, Adel Kara Slimane <adel.ks@zegrapher.com>
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
#include <initializer_list>
#include <ranges>
#include <type_traits>
#include <utility>

#include <zecalculator/external/flat_map.h>

namespace zc {
namespace eval {

struct ObjectCache {

  constexpr static size_t default_buffer_size = 32;

  ObjectCache(size_t buffer_size = default_buffer_size);

  ObjectCache(std::initializer_list<double> keys,
              std::initializer_list<double> vals,
              size_t object_revision,
              size_t buffer_size = default_buffer_size);

  /// @brief Fills the cache with a given range of keys and range of values
  /// @note does not expect the keys range to be sorted
  template <std::ranges::viewable_range Keys, std::ranges::viewable_range Values>
    requires std::is_same_v<std::ranges::range_value_t<Keys>, double>
             and std::is_same_v<std::ranges::range_value_t<Values>, double>
  ObjectCache(Keys&& keys,
              Values&& values,
              size_t object_revision,
              size_t buffer_size = default_buffer_size);

  ObjectCache(ObjectCache&&) = default;
  ObjectCache(const ObjectCache&) = default;

  size_t get_buffer_size() const { return buffer_size; }

  size_t get_cached_revision() const { return cached_object_revision; }

  /// @brief update the max buffer size to this size
  /// @note if it's smaller than the previously set one, oldest values are popped out of the cache
  void set_buffer_size(size_t new_buffer_size);

  /// @brief insert new point to cache
  /// @param revision: revision of the cached object
  /// @note key cannot be "NaN"
  void insert(size_t object_revision, double key, double value);

  /// @brief clears the cache entirely
  void clear();

  /// @returns the cached value, if it exists
  std::optional<double> get_value(size_t object_revision, double key);

  const std::flat_map<double, double>& get_cache() const { return cache; }

protected:
  /// @note added 'std::monostate' argument just so we use it as a delegate constructor for both
  ///       std::initializer_list<double> and the rest of the containers
  template <std::ranges::viewable_range Keys, std::ranges::viewable_range Values>
    requires std::is_same_v<std::ranges::range_value_t<Keys>, double>
             and std::is_same_v<std::ranges::range_value_t<Values>, double>
  ObjectCache(std::in_place_t, Keys&& keys, Values&& values, size_t object_revision, size_t buffer_size);

  std::flat_map<double, double> cache;

  /// @brief indices of cache, sorted from oldest to newest
  std::deque<size_t> age_sorted_indices;

  size_t buffer_size = 0;
  size_t cached_object_revision = 0;
};

}
}
