#pragma once

#include <string_view>
#include <cassert>

namespace zc {


/// @brief This class is some kind of meta std::string_view
///        it contains the beginning index and the size of a substring
///        without saving a reference/pointer/copy to the original string
///        a substring can be retrieved by giving it the original expression
struct SubstrInfo
{
  constexpr static SubstrInfo from_views(std::string_view substr, std::string_view full_str)
  {
    assert(full_str.data() <= substr.data()
           and substr.data() + substr.size() <= full_str.data() + full_str.size());

    return SubstrInfo{.begin = size_t(substr.data() - full_str.data()), .size = substr.size()};
  };

  ///@brief begin position in the original string
  size_t begin = 0;

  ///@brief substring size
  size_t size = 0;

  bool operator == (const SubstrInfo& other) const = default;
};

inline SubstrInfo operator + (const SubstrInfo& a, const SubstrInfo& b)
{
  const size_t begin = std::min(a.begin, b.begin);
  return SubstrInfo
  {
    .begin = begin, .size = std::max(a.begin + a.size, b.begin + b.size) - begin
  };
}

}
