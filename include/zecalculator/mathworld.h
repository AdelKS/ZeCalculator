#pragma once

#include <zecalculator/builtin_unary_functions.h>
#include <zecalculator/builtin_binary_functions.h>
#include <zecalculator/utils/slotted_vector.h>
#include <zecalculator/global_constant.h>

#include <unordered_map>
#include <variant>
#include <string>

namespace zc {

class MathWorld
{
public:
  using MathObject
    = std::variant<std::monostate, CppUnaryFunction, CppBinaryFunction, GlobalConstant>;

  /// @brief default world that contains the usual functions (cos, sin ...)
  static const MathWorld default_world;

  MathWorld() : MathWorld(default_world) {}

  template <size_t unary_fun_num, size_t binary_fun_num, size_t global_var_num>
  MathWorld(
    const std::array<std::pair<std::string_view, CppUnaryFunction>, unary_fun_num>& unary_funs,
    const std::array<std::pair<std::string_view, CppBinaryFunction>, binary_fun_num>& binary_funs,
    const std::array<std::pair<std::string_view, double>, global_var_num>& global_vars)
  {
    for(auto&& [name, f]: unary_funs)
      register_cpp_function(name, f);

    for(auto&& [name, f]: binary_funs)
      register_cpp_function(name, f);

    for(auto&& [name, value]: global_vars)
      add_global_constant(name, value);
  }

  // Types of the objects
  enum ObjectType
  {
    NOT_REGISTERED,
    CPP_UNARY_FUNCTION,
    CPP_BINARY_FUNCTION,
    GLOBAL_CONSTANT
  };

  MathObject get_math_object(std::string_view name) const
  {
    auto it = inventory.find(name);
    const auto& [type, index] = (it != inventory.end()) ? it->second
                                                        : std::make_pair(NOT_REGISTERED, size_t(0));

    /// TODO: bounds check can be removed when code is stable, aka use operator[]
    switch(type)
    {
      case ObjectType::CPP_UNARY_FUNCTION:
        return unary_cpp_functions.at(index);
      case ObjectType::CPP_BINARY_FUNCTION:
        return binary_cpp_functions.at(index);
      case ObjectType::GLOBAL_CONSTANT:
        return global_variables.at(index);
      default:
        return std::monostate();
    }
  }

  /// @brief register cpp function
  void register_cpp_function(std::string_view name, CppUnaryFunction f)
  {
    inventory.insert({std::string(name), {CPP_UNARY_FUNCTION, unary_cpp_functions.push(f)}});
  }

  /// @brief register cpp function
  void register_cpp_function(std::string_view name, CppBinaryFunction f)
  {
    inventory.insert({std::string(name), {CPP_BINARY_FUNCTION, binary_cpp_functions.push(f)}});
  }

  /// @brief add global variable
  void add_global_constant(std::string_view name, const double init_value)
  {
    // check if it's not already registered
    auto cst = get_global_constant(name);
    if (cst)
      const_cast<GlobalConstant&>(cst->get()).value = init_value;
    else inventory.insert({std::string(name), {GLOBAL_CONSTANT, global_variables.push(GlobalConstant{init_value})}});
  }

  std::optional<std::reference_wrapper<const GlobalConstant>> get_global_constant(std::string_view name) const
  {
    auto it = inventory.find(name);
    const auto& [type, index] = (it != inventory.end()) ? it->second
                                                        : std::make_pair(NOT_REGISTERED, size_t(0));

    /// TODO: bounds check can be removed when code is stable, aka use operator[]
    if (type == GLOBAL_CONSTANT)
      return std::cref(global_variables.at(index));
    else return {};
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

  SlottedVector<CppUnaryFunction> unary_cpp_functions;
  SlottedVector<CppBinaryFunction> binary_cpp_functions;
  SlottedVector<GlobalConstant> global_variables;
};

extern MathWorld global_world;

}
