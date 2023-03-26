#pragma once

#include <zecalculator/builtin_unary_functions.h>
#include <zecalculator/builtin_binary_functions.h>


#include <unordered_map>
#include <variant>
#include <string>

namespace zc {

class MathWorld
{
public:

  using MathObject = std::variant<std::monostate, CppUnaryFunction, CppBinaryFunction>;

  MathWorld()
  {
    // populate inventory with builtin functions
    auto register_builtin_functions = [&](const auto& builtin_functions, const ObjectType function_type )
    {
      size_t i = 0 ;
      for (const auto &[name, function]: builtin_functions)
      {
        inventory.insert({std::string(name), {function_type, i}});
        i++;
      }
    };

    register_builtin_functions(builtin_unary_functions,  CPP_UNARY_FUNCTION);
    register_builtin_functions(builtin_binary_functions, CPP_BINARY_FUNCTION);
  }

  // Types of the objects
  enum ObjectType
  {
    NOT_REGISTERED,
    CPP_UNARY_FUNCTION,
    CPP_BINARY_FUNCTION
  };

  MathObject get_math_object(std::string_view name) const
  {
    auto it = inventory.find(name);
    const auto& [type, index] = (it != inventory.end()) ? it->second
                                                        : std::make_pair(NOT_REGISTERED, size_t(0));

    switch(type)
    {
      case ObjectType::CPP_UNARY_FUNCTION:
        return builtin_unary_functions[index].second;
      case ObjectType::CPP_BINARY_FUNCTION:
        return builtin_binary_functions[index].second;
      default:
        return std::monostate();
    }
  }

protected:

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

extern MathWorld global_world;

}
