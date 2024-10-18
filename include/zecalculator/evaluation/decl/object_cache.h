#pragma once

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
  ObjectCache(size_t buffer_size = 8);

  ObjectCache(std::initializer_list<double> keys,
              std::initializer_list<double> vals,
              size_t buffer_size = 8);

  /// @brief Fills the cache with a given range of keys and range of values
  /// @note does not expect the keys range to be sorted
  template <std::ranges::viewable_range Keys, std::ranges::viewable_range Values>
    requires std::is_same_v<std::ranges::range_value_t<Keys>, double>
             and std::is_same_v<std::ranges::range_value_t<Values>, double>
  ObjectCache(Keys&& keys, Values&& values, size_t buffer_size = 32);

  ObjectCache(ObjectCache&&) = default;
  ObjectCache(const ObjectCache&) = default;

  /// @brief update the max buffer size to this size
  /// @note if it's smaller than the previously set one, oldest values are popped out of the cache
  void set_buffer_size(size_t new_buffer_size);

  /// @brief insert new point to cache
  /// @note key cannot be "NaN"
  void insert(double key, double value);

  const std::flat_map<double, double>& get_cache() const { return cache; }

protected:
  /// @note added 'std::monostate' argument just so we use it as a delegate constructor for both
  ///       std::initializer_list<double> and the rest of the containers
  template <std::ranges::viewable_range Keys, std::ranges::viewable_range Values>
    requires std::is_same_v<std::ranges::range_value_t<Keys>, double>
             and std::is_same_v<std::ranges::range_value_t<Values>, double>
  ObjectCache(std::in_place_t, Keys&& keys, Values&& values, size_t buffer_size);

  size_t buffer_size = 0;
  std::flat_map<double, double> cache;

  /// @brief indices of cache, sorted from oldest to newest
  std::deque<size_t> age_sorted_indices;
};

}
}
