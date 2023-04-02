#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>
#include <optional>
#include <stack>

namespace zc {

/// @brief a vector of T, where each element keeps its index
///        during its whole lifetime
template <class T>
class SlottedVector: protected std::vector<std::optional<T>>
{
  using Parent = std::vector<std::optional<T>>;

public:
  SlottedVector() = default;

  using Parent::size;

  /// @brief finds the a free slot, puts 'val' in it, then returns the slot index
  size_t push(T val)
  {
    const size_t slot = pop_free_slot();
    Parent::operator[](slot) = val;
    return slot;
  }

  ///@brief frees the slot 'slot'
  void pop(const size_t slot)
  {
    (*this)[slot].reset();
    free_slots.push(slot);
  }

  /// @brief returns the element T at 'slot', bounds checked
  const T& at(size_t slot) const
  {
    return Parent::at(slot).value();
  }

  /// @brief returns the element T at 'slot'
  const T& operator [] (size_t slot) const
  {
    return *Parent::operator [] (slot);
  }

    /// @brief returns the element T at 'slot'
  T& operator [] (size_t slot)
  {
    return *Parent::operator [] (slot);
  }

protected:

  size_t pop_free_slot()
  {
    if (free_slots.empty())
    {
      const size_t slot = size();
      Parent::push_back(std::optional<T>());
      return slot;
    }
    else
    {
      const size_t slot = free_slots.top();
      free_slots.pop();
      return slot;
    }
  }

  std::stack<size_t> free_slots;
};

}
