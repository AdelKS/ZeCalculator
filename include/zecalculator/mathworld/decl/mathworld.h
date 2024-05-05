#pragma once

/****************************************************************************
**  Copyright (c) 2023, Adel Kara Slimane <adel.ks@zegrapher.com>
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

#include <zecalculator/error.h>
#include <zecalculator/math_objects/builtin.h>
#include <zecalculator/math_objects/global_constant.h>
#include <zecalculator/math_objects/object_list.h>
#include <zecalculator/math_objects/decl/dyn_math_object.h>
#include <zecalculator/utils/name_map.h>
#include <zecalculator/utils/refs.h>
#include <zecalculator/utils/slotted_deque.h>
#include <zecalculator/utils/tuple.h>
#include <zecalculator/utils/utils.h>

#include <string>
#include <string_view>

namespace zc {

template <parsing::Type type>
class MathWorld;

namespace fast {
  using MathWorld = zc::MathWorld<parsing::Type::FAST>;
}

namespace rpn {
  using MathWorld = zc::MathWorld<parsing::Type::RPN>;
}

class Ok {};
class UnregisteredObject {};

template <parsing::Type type>
class MathWorld
{
public:

  /// @brief default constructor that defines the usual functions and global constants
  MathWorld();

  /// @brief get object from name, the underlying type is to be dynamically resolved at runtime
  /// @note const version
  const DynMathObject<type>* get(std::string_view name) const;

  /// @brief get object from name, the underlying type is to be dynamically resolved at runtime
  DynMathObject<type>* get(std::string_view name);

  /// @brief get object 'ObjectType' from name, if it exists, nullptr otherwise
  template <class ObjectType>
    requires tuple_contains_v<MathObjects<type>, ObjectType>
  ObjectType* get(std::string_view name);

  /// @brief get object 'ObjectType' from name, if it exists, nullptr otherwise
  /// @note const version
  template <class ObjectType>
    requires tuple_contains_v<MathObjects<type>, ObjectType>
  const ObjectType* get(std::string_view name) const;

  /// @brief get object 'ObjectType' from id
  /// @note ids are not unique, they live in different sets between ObjectTypes
  template <class ObjectType>
    requires tuple_contains_v<MathObjects<type>, ObjectType>
  ObjectType& get(size_t id);

  /// @brief get object 'ObjectType' from id
  /// @note const version
  template <class ObjectType>
    requires tuple_contains_v<MathObjects<type>, ObjectType>
  const ObjectType& get(size_t id) const;

  /// @brief returns a handle to a new math object
  /// @note  the variant within contains the Unknown alternative
  DynMathObject<type>& add();

  /// @brief Add new object with given definition, see 'redefine()' for more information
  template <class InterpretAs = DynMathObject<type>>
    requires (tuple_contains_v<MathEqObjects<type>, InterpretAs>
              or std::is_same_v<DynMathObject<type>, InterpretAs>)
  DynMathObject<type>& add(std::string definition);

  /// @brief add cpp function
  template <size_t args_num>
    requires (args_num <= max_func_args)
  DynMathObject<type>& add(std::string name, CppMathFunctionPtr<args_num> cpp_f);

  /// @brief redefine 'obj' with 'definition' and intepret the definition as 'InterpretAs' type
  /// @note the default 'DynMathObject<type>>' type means "auto", i.e. try to guess the object type
  /// @returns the same reference: the variant within may contain a different alternative
  /// @example in auto, definition := "y = 1.2" returns a DynMathObject that contains a GlobalConstant
  /// @example in auto, definition := "y = f(1.2)" returns a DynMathObject that contains a a GlobalVariable
  /// @example in auto, definition := "y(x) = cos(x)" returns a DynMathObject that contains a a Function
  template <class InterpretAs = DynMathObject<type>>
    requires (tuple_contains_v<MathEqObjects<type>, InterpretAs>
              or std::is_same_v<DynMathObject<type>, InterpretAs>)
  DynMathObject<type>& redefine(DynMathObject<type>& obj, std::string definition);

  /// @brief redefine math object as cpp function
  template <size_t args_num>
    requires (args_num <= max_func_args)
  DynMathObject<type>& redefine(DynMathObject<type>& obj, std::string name, CppMathFunctionPtr<args_num> cpp_f);

  /// @brief says if an object with the given name exists within the world
  bool contains(std::string_view name) const;

  /// @brief evaluates a given expression within this world
  tl::expected<double, Error> evaluate(std::string expr);

  /// @brief delete object given by pointer
  /// @returns Ok if the deletion was successful, UnregisteredObject otherwise
  ///          when the pointed-to object is not handled by this instance of MathWorld
  template <class ObjectType>
    requires(tuple_contains_v<MathObjects<type>, ObjectType>)
  tl::expected<Ok, UnregisteredObject> erase(ObjectType& obj);

  /// @brief return the direct reverse dependencies, aka objects that depend directly on 'name'
  deps::Deps direct_revdeps(std::string_view name) const;

  /// @brief delete object given by pointer
  /// @returns Ok if the deletion was successful, UnregisteredObject otherwise
  ///          when the pointed-to object is not handled by this instance of MathWorld
  tl::expected<Ok, UnregisteredObject> erase(DynMathObject<type>& obj);

  /// @brief delete object given by name
  /// @returns Ok if the deletion was successful, UnregisteredObject otherwise
  ///          when no registered object has that given name
  tl::expected<Ok, UnregisteredObject> erase(size_t slot);

    /// @brief delete object given by name
  /// @returns Ok if the deletion was successful, UnregisteredObject otherwise
  ///          when no registered object has that given name
  tl::expected<Ok, UnregisteredObject> erase(const std::string& name);

  /// @brief maximum recursion depth to reach before returning an error
  size_t max_recursion_depth = 20;


protected:

  /// @brief returns a handle to a new math object
  /// @note  the variant within contains the Unknown alternative
  DynMathObject<type>& add(size_t slot);

  /// @brief Add new object with given definition, see 'redefine()' for more information
  template <class InterpretAs = DynMathObject<type>>
    requires (tuple_contains_v<MathEqObjects<type>, InterpretAs>
              or std::is_same_v<DynMathObject<type>, InterpretAs>)
  DynMathObject<type>& add(std::string definition, size_t slot);

  /// @brief add cpp function
  template <size_t args_num>
    requires (args_num <= max_func_args)
  DynMathObject<type>& add(std::string name, CppMathFunctionPtr<args_num> cpp_f, size_t slot);

  /// @brief checks that this object has actually been allocated in this world
  bool sanity_check(const DynMathObject<type>& obj);

  /// @brief parse all Function-based object that directly depends on obj_name
  void rebind_direct_revdeps_of(const std::string& obj_name);

  /// @brief maps an object name to its slot
  name_map<size_t> inventory;

  /// @brief two uses: tracks 1. all objects handled by this world, 2. their name if they have one (empty otherwise)
  SlottedDeque<std::string> object_names;

  SlottedDeque<DynMathObject<type>> math_objects;

};

}
