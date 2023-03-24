#pragma once

#include <zecalculator/builtin_function.h>

#include <functional>
#include <string_view>
#include <unordered_map>
#include <cmath>
#include <variant>
#include <optional>
#include <string>

namespace zc {

class MathWorld
{
public:

  using MathObject = std::variant<std::monostate, BuiltinFunction>;

  MathWorld()
  {
    // populate inventory with builtin functions
    size_t i = 0 ;
    for (const auto &[name, function]: builtin_functions)
    {
      inventory.insert({std::string(name), {ObjectType::BUILTIN_FUNCTION, i}});
      i++;
    }
  }

  // Types of the objects
  enum ObjectType {NOT_REGISTERED, BUILTIN_FUNCTION};

  MathObject get_math_object(std::string_view name) const
  {
    auto it = inventory.find(name);
    const auto& [type, index] = (it != inventory.end()) ? it->second
                                                        : std::make_pair(NOT_REGISTERED, size_t(0));

    switch(type)
    {
      case ObjectType::BUILTIN_FUNCTION:
        return builtin_functions[index].second;
      default:
        return std::monostate();
    }
  }

protected:

  // we save the names along with the function pointers for convenience
  // we could save only the function pointers, and the names only in the inventory
  static constexpr std::array<std::pair<std::string_view, BuiltinFunction>, 31> builtin_functions = {{
    {"cos",   static_cast<BuiltinFunction>(std::cos)},
    {"sin",   static_cast<BuiltinFunction>(std::sin)},
    {"tan",   static_cast<BuiltinFunction>(std::tan)},

    {"acos",  static_cast<BuiltinFunction>(std::acos)},
    {"asin",  static_cast<BuiltinFunction>(std::asin)},
    {"atan",  static_cast<BuiltinFunction>(std::atan)},

    {"cosh",  static_cast<BuiltinFunction>(std::cosh)},
    {"sinh",  static_cast<BuiltinFunction>(std::sinh)},
    {"tanh",  static_cast<BuiltinFunction>(std::tanh)},

    {"ch",    static_cast<BuiltinFunction>(std::cosh)},
    {"sh",    static_cast<BuiltinFunction>(std::sinh)},
    {"th",    static_cast<BuiltinFunction>(std::tanh)},

    {"acosh", static_cast<BuiltinFunction>(std::acosh)},
    {"asinh", static_cast<BuiltinFunction>(std::asinh)},
    {"atanh", static_cast<BuiltinFunction>(std::atanh)},

    {"ach",   static_cast<BuiltinFunction>(std::acosh)},
    {"ash",   static_cast<BuiltinFunction>(std::asinh)},
    {"ath",   static_cast<BuiltinFunction>(std::atanh)},

    {"sqrt",  static_cast<BuiltinFunction>(std::sqrt)},
    {"log",   static_cast<BuiltinFunction>(std::log10)},
    {"lg",    static_cast<BuiltinFunction>(std::log2)},
    {"ln",    static_cast<BuiltinFunction>(std::log)},
    {"abs",   static_cast<BuiltinFunction>(std::abs)},
    {"exp",   static_cast<BuiltinFunction>(std::exp)},
    {"floor", static_cast<BuiltinFunction>(std::floor)},
    {"ceil",  static_cast<BuiltinFunction>(std::ceil)},
    {"erf",   static_cast<BuiltinFunction>(std::erf)},
    {"erfc",  static_cast<BuiltinFunction>(std::erfc)},
    {"gamma", static_cast<BuiltinFunction>(std::tgamma)},
    {"Γ",     static_cast<BuiltinFunction>(std::tgamma)},
  }};

  struct string_hash
  {
      using hash_type = std::hash<std::string_view>;
      using is_transparent = void;

      std::size_t operator()(const char* str) const        { return hash_type{}(str); }
      std::size_t operator()(std::string_view str) const   { return hash_type{}(str); }
      std::size_t operator()(std::string const& str) const { return hash_type{}(str); }
  };

  /// @brief maps an object name to its type and ID (index within the container that holds it)
  std::unordered_map<std::string, std::pair<ObjectType, size_t>, string_hash, std::equal_to<>> inventory;
};

extern MathWorld mathWorld;

}
