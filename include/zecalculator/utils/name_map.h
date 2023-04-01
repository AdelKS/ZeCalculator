#pragma once

#include <unordered_map>
#include <string_view>
#include <string>

namespace zc {

struct string_hash
{
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;

    std::size_t operator()(const char* str) const        { return hash_type{}(str); }
    std::size_t operator()(std::string_view str) const   { return hash_type{}(str); }
    std::size_t operator()(std::string const& str) const { return hash_type{}(str); }
};

/// @brief a map that accepts searching a string_view key
template <class T>
using name_map = std::unordered_map<std::string, T, string_hash, std::equal_to<>>;

}
