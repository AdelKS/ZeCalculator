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

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iterator>
#include <limits>
#include <utility>
#include <zecalculator/evaluation/decl/object_cache.h>

namespace zc {
namespace eval {

inline ObjectCache::ObjectCache(size_t new_buffer_size)
{
  set_buffer_size(new_buffer_size);
}

/// @brief update the max buffer size to this size
/// @note if it's smaller than the previously set one, oldest values are popped out of the cache
inline void ObjectCache::set_buffer_size(size_t new_buffer_size)
{
  if (new_buffer_size == buffer_size) [[unlikely]]
    return;

  assert(age_sorted_indices.size() == cache.size());
  assert(cache.size() <= buffer_size);

  if (new_buffer_size > cache.size())
  {
    auto [keys, values] = std::move(cache).extract();
    // we allow ourselves to go +1 above buffer size in inserts, for a tad leaner code there
    keys.reserve(new_buffer_size + 1);
    values.reserve(new_buffer_size + 1);
    cache.replace(std::move(keys), std::move(values));
  }
  else if (new_buffer_size < cache.size())
  {
    size_t elements_to_pop = age_sorted_indices.size() - new_buffer_size;

    for (size_t i = 0 ; i != elements_to_pop; i++)
      pop_oldest();
  }

  buffer_size = new_buffer_size;
}

inline ObjectCache::ObjectCache(std::initializer_list<double> keys,
                                std::initializer_list<double> vals,
                                size_t object_revision,
                                size_t buffer_size)
  : ObjectCache(std::in_place_t{}, keys, vals, object_revision, buffer_size)
{}

template <std::ranges::viewable_range Keys, std::ranges::viewable_range Values>
  requires std::is_same_v<std::ranges::range_value_t<Keys>, double>
           and std::is_same_v<std::ranges::range_value_t<Values>, double>
ObjectCache::ObjectCache(Keys&& keys, Values&& values, size_t object_revision, size_t buffer_size)
  : ObjectCache(std::in_place_t{},
                std::forward<Keys>(keys),
                std::forward<Values>(values),
                object_revision,
                buffer_size)
{
}

template <std::ranges::viewable_range Keys, std::ranges::viewable_range Values>
  requires std::is_same_v<std::ranges::range_value_t<Keys>, double>
           and std::is_same_v<std::ranges::range_value_t<Values>, double>
ObjectCache::ObjectCache(
  std::in_place_t, Keys&& keys, Values&& values, size_t object_revision, size_t buffer_size)
  : ObjectCache(buffer_size)
{
  auto kit = keys.begin();
  auto vit = values.begin();
  for (; kit != keys.end() and vit != values.end() ; kit++, vit++)
  {
    insert(object_revision, *kit, *vit);
  }

  assert(kit == keys.end() and vit == values.end());
}

inline std::optional<double> ObjectCache::get_value(size_t object_revision, double key)
{
  if (cached_object_revision != object_revision)
    return {};

  if (auto value_it = cache.find(key); value_it != cache.end())
    return value_it->second;
  else return {};
}

inline void ObjectCache::clear()
{
  cache.clear();
  age_sorted_indices.clear();
}

inline void ObjectCache::insert(size_t object_revision, double key, double value)
{
  // we do not expect "NaN" keys
  assert(not std::isnan(key));

  if (cached_object_revision != object_revision)
  {
    clear();
    cached_object_revision = object_revision;
  }

  auto [it, is_inserted] = cache.insert_or_assign(key, value);

  int distance = std::distance(cache.begin(), it);
  assert(distance >= 0);
  size_t insertion_index = distance;

  if (is_inserted)
  {
    // All indices that comes after the insertion index need to be incremented
    std::ranges::for_each(age_sorted_indices, [&](size_t& i){ if (i >= insertion_index) i++; });

    age_sorted_indices.push_back(insertion_index);

    if (cache.size() > buffer_size)
      pop_oldest();
  }
  else
  {
    // the index got replaced with a new value
    assert(std::ranges::count(age_sorted_indices, insertion_index) == 1);
    std::erase(age_sorted_indices, insertion_index);
    age_sorted_indices.push_back(insertion_index);
  }
}

inline void ObjectCache::pop_oldest()
{
  if (cache.size() == 0)
    return;

  size_t pop_index = age_sorted_indices.front();
  age_sorted_indices.pop_front();
  std::ranges::for_each(age_sorted_indices, [&](size_t& i){ if (i >= pop_index) i--; });

  auto&& [keys, values] = std::move(cache).extract();
  keys.erase(keys.begin() + pop_index);
  values.erase(values.begin() + pop_index);

  cache.replace(std::move(keys), std::move(values));
}

}
}
