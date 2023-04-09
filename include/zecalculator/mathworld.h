#pragma once

#include <stdexcept>
#include <zecalculator/builtin_binary_functions.h>
#include <zecalculator/builtin_unary_functions.h>
#include <zecalculator/global_constant.h>
#include <zecalculator/utils/name_map.h>
#include <zecalculator/utils/slotted_vector.h>
#include <zecalculator/utils/optional_ref.h>
#include <zecalculator/function.h>

#include <unordered_map>
#include <variant>
#include <string>

namespace zc {

class MathWorld
{
public:
  using MathObject
    = std::variant<std::monostate, CppUnaryFunction, CppBinaryFunction, GlobalConstant, Function>;

  class name_already_taken: public std::runtime_error
  {
    using std::runtime_error::runtime_error;
  };

  class WorldFunction
  {
    public:
      Function& operator * ()
      {
        return world.get_function(id);
      }

      Function& operator -> ()
      {
        return **this;
      }

    protected:
      WorldFunction(MathWorld& world, size_t id)
        : world(world), id(id) {}

      MathWorld& world;
      size_t id;
      friend MathWorld;
  };

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
    GLOBAL_CONSTANT,
    FUNCTION,
  };

  MathObject get_math_object(std::string_view name) const
  {
    auto it = inventory.find(name);
    const auto& [type, index] = (it != inventory.end()) ? it->second
                                                        : std::make_pair(NOT_REGISTERED, size_t(0));

    /// TODO: bounds check can be removed when code is stable, aka use operator[]
    switch(type)
    {
      case CPP_UNARY_FUNCTION:
        return unary_cpp_functions.at(index);
      case CPP_BINARY_FUNCTION:
        return binary_cpp_functions.at(index);
      case GLOBAL_CONSTANT:
        return global_variables.at(index);
      case FUNCTION:
        return functions.at(index);
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

  optional_ref<const GlobalConstant> get_global_constant(std::string_view name) const
  {
    auto it = inventory.find(name);
    const auto& [type, index] = (it != inventory.end()) ? it->second
                                                        : std::make_pair(NOT_REGISTERED, size_t(0));

    /// TODO: bounds check can be removed when code is stable, aka use operator[]
    if (type == GLOBAL_CONSTANT)
      return std::cref(global_variables.at(index));
    else return {};
  }

  /// @brief says if an object with the given name exists within the world
  bool contains(std::string_view name) const
  {
    return inventory.find(name) != inventory.end();
  }

  /// @brief add a functions
  /// @returns a reference to the function object, if it got created
  /// @note if another object has the same name, no function is created and an exception is raised
  WorldFunction add_function(std::string_view name);

  Function& get_function(size_t id)
  {
    return functions[id];
  }

protected:

  /// @brief maps an object name to its type and ID (index within the container that holds it)
  name_map<std::pair<ObjectType, size_t>> inventory;

  SlottedVector<CppUnaryFunction> unary_cpp_functions;
  SlottedVector<CppBinaryFunction> binary_cpp_functions;
  SlottedVector<GlobalConstant> global_variables;
  SlottedVector<Function> functions;
};

extern MathWorld global_world;

}
