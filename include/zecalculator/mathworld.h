#pragma once

#include <zecalculator/builtin_unary_functions.h>

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

  using MathObject = std::variant<std::monostate, BuiltinUnaryFunction>;

  MathWorld()
  {
    // populate inventory with builtin functions
    size_t i = 0 ;
    for (const auto &[name, function]: builtin_unary_functions)
    {
      inventory.insert({std::string(name), {ObjectType::BUILTIN_UNARY_FUNCTION, i}});
      i++;
    }
  }

  // Types of the objects
  enum ObjectType {NOT_REGISTERED, BUILTIN_UNARY_FUNCTION};

  MathObject get_math_object(std::string_view name) const
  {
    auto it = inventory.find(name);
    const auto& [type, index] = (it != inventory.end()) ? it->second
                                                        : std::make_pair(NOT_REGISTERED, size_t(0));

    switch(type)
    {
      case ObjectType::BUILTIN_UNARY_FUNCTION:
        return builtin_unary_functions[index].second;
      default:
        return std::monostate();
    }
  }

protected:

  // we save the names along with the function pointers for convenience
  // we could save only the function pointers, and the names only in the inventory
  static constexpr std::array<std::pair<std::string_view, BuiltinUnaryFunction>, 31> builtin_unary_functions = {{
    {"cos",   static_cast<BuiltinUnaryFunction>(std::cos)},
    {"sin",   static_cast<BuiltinUnaryFunction>(std::sin)},
    {"tan",   static_cast<BuiltinUnaryFunction>(std::tan)},

    {"acos",  static_cast<BuiltinUnaryFunction>(std::acos)},
    {"asin",  static_cast<BuiltinUnaryFunction>(std::asin)},
    {"atan",  static_cast<BuiltinUnaryFunction>(std::atan)},

    {"cosh",  static_cast<BuiltinUnaryFunction>(std::cosh)},
    {"sinh",  static_cast<BuiltinUnaryFunction>(std::sinh)},
    {"tanh",  static_cast<BuiltinUnaryFunction>(std::tanh)},

    {"ch",    static_cast<BuiltinUnaryFunction>(std::cosh)},
    {"sh",    static_cast<BuiltinUnaryFunction>(std::sinh)},
    {"th",    static_cast<BuiltinUnaryFunction>(std::tanh)},

    {"acosh", static_cast<BuiltinUnaryFunction>(std::acosh)},
    {"asinh", static_cast<BuiltinUnaryFunction>(std::asinh)},
    {"atanh", static_cast<BuiltinUnaryFunction>(std::atanh)},

    {"ach",   static_cast<BuiltinUnaryFunction>(std::acosh)},
    {"ash",   static_cast<BuiltinUnaryFunction>(std::asinh)},
    {"ath",   static_cast<BuiltinUnaryFunction>(std::atanh)},

    {"sqrt",  static_cast<BuiltinUnaryFunction>(std::sqrt)},
    {"log",   static_cast<BuiltinUnaryFunction>(std::log10)},
    {"lg",    static_cast<BuiltinUnaryFunction>(std::log2)},
    {"ln",    static_cast<BuiltinUnaryFunction>(std::log)},
    {"abs",   static_cast<BuiltinUnaryFunction>(std::abs)},
    {"exp",   static_cast<BuiltinUnaryFunction>(std::exp)},
    {"floor", static_cast<BuiltinUnaryFunction>(std::floor)},
    {"ceil",  static_cast<BuiltinUnaryFunction>(std::ceil)},
    {"erf",   static_cast<BuiltinUnaryFunction>(std::erf)},
    {"erfc",  static_cast<BuiltinUnaryFunction>(std::erfc)},
    {"gamma", static_cast<BuiltinUnaryFunction>(std::tgamma)},
    {"Γ",     static_cast<BuiltinUnaryFunction>(std::tgamma)},
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
