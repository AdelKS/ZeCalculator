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
    keys.reserve(buffer_size);
    values.reserve(buffer_size);
    cache.replace(std::move(keys), std::move(values));
  }
  else if (new_buffer_size < cache.size())
  {
    size_t elements_to_pop = age_sorted_indices.size() - new_buffer_size;

    // insert values that will be appended to the end of the flat_map
    // so we can shrink it without issues: those newly pushed values will be popped
    // and the old values that actually need to be removed will be by the insert() calls
    for (size_t i = 0 ; i != elements_to_pop; i++)
      insert(cached_object_revision, std::numeric_limits<double>::max(), 0.);

    auto [keys, values] = std::move(cache).extract();
    keys.resize(new_buffer_size);
    values.resize(new_buffer_size);
    age_sorted_indices.resize(new_buffer_size);

    cache.replace(std::move(keys), std::move(values));
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


  if (cache.size() < buffer_size) [[unlikely]]
  {
    // we haven't filled the buffer yet
    auto [it, is_inserted] = cache.insert_or_assign(key, value);

    int distance = std::distance(cache.begin(), it);
    assert(distance >= 0);
    size_t insertion_index = distance;

    if (is_inserted)
    {
      // All indices that comes after the insertion index need to be incremented
      std::ranges::for_each(age_sorted_indices, [&](size_t& i){ if (i >= insertion_index) i++; });

      // element inserted, since the buffer isn't full yet, we just push back this index
      age_sorted_indices.push_back(insertion_index);
    }
    else
    {
      // the index got replaced with a new value
      assert(std::ranges::count(age_sorted_indices, insertion_index) == 1);
      std::erase(age_sorted_indices, insertion_index);
      age_sorted_indices.push_back(insertion_index);
    }
  }
  else
  {
    // buffer is full now

    auto [keys, values] = std::move(cache).extract();

    assert(not age_sorted_indices.empty());
    assert(age_sorted_indices.size() == keys.size());

    auto it = std::ranges::lower_bound(keys, key);

    size_t erase_index = age_sorted_indices.front();
    age_sorted_indices.pop_front();

    assert(std::ranges::count(age_sorted_indices, erase_index) == 0); // all indices should be unique

    int distance = std::distance(keys.begin(), it);
    assert(distance >= 0);

    size_t insertion_index = distance;

    if (insertion_index == erase_index) [[unlikely]]
    {
      *it = key; //
      values[insertion_index] = value;

      age_sorted_indices.push_back(insertion_index);
    }
    else if (insertion_index < erase_index)
    {
      std::ranges::for_each(age_sorted_indices, [&](size_t& i){ if (insertion_index <= i and i < erase_index) i++; });

      double extracted_key = std::nan(""), extracted_value = std::nan("");
      for (size_t i = 0 ; i != keys.size() ; i++)
      {
        if (i == insertion_index)
        {
          extracted_key = keys[i];
          extracted_value = values[i];

          keys[i] = key;
          values[i] = value;
        }
        else if (insertion_index < i and i < erase_index)
        {
          std::swap(keys[i], extracted_key);
          std::swap(values[i], extracted_value);
        }
        else if (i == erase_index)
        {
          keys[i] = extracted_key;
          values[i] = extracted_value;
        }
      }

      // needs to come at the end because we update values 'age_sorted_indices' to move them to the right or left
      // depending on how things got inserted/deleted
      age_sorted_indices.push_back(insertion_index);
    }
    else // erase_index < insertion_index
    {
      std::ranges::for_each(age_sorted_indices, [&](size_t& i){ if (erase_index < i and i < insertion_index) i--; });

      for (size_t i = 0 ; i != keys.size() ; i++)
      {
        if (erase_index <= i and i < insertion_index-1)
        {
          keys[i] = keys[i+1];
          values[i] = values[i+1];
        }
        if (i == insertion_index-1)
        {
          keys[i] = key;
          values[i] = value;
        }
      }

      // needs to come at the end because we update values 'age_sorted_indices' to move them to the right or left
      // depending on how things got inserted/deleted
      age_sorted_indices.push_back(insertion_index-1);
    }

    cache.replace(std::move(keys), std::move(values));
  }
}

}
}
