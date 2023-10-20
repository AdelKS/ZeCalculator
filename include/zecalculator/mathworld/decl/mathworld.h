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
#include <zecalculator/math_objects/cpp_binary_functions.h>
#include <zecalculator/math_objects/cpp_unary_functions.h>
#include <zecalculator/math_objects/global_constant.h>
#include <zecalculator/math_objects/object_list.h>
#include <zecalculator/utils/name_map.h>
#include <zecalculator/utils/refs.h>
#include <zecalculator/utils/slotted_deque.h>
#include <zecalculator/utils/tuple.h>
#include <zecalculator/utils/utils.h>

#include <string>
#include <variant>
#include <string_view>

namespace zc {

template <parsing::Type type>
class MathWorld;

namespace ast {
  using MathWorld = zc::MathWorld<parsing::Type::AST>;
}

namespace rpn {
  using MathWorld = zc::MathWorld<parsing::Type::RPN>;
}


struct NameError
{
  enum Type {ALREADY_TAKEN, INVALID_FORMAT};

  static NameError already_taken(std::string_view name)
  {
    return NameError{.type = ALREADY_TAKEN, .name = std::string(name)};
  }

  static NameError invalid_format(std::string_view name)
  {
    return NameError{.type = INVALID_FORMAT, .name = std::string(name)};
  }

  Type type;
  std::string name;
};


class UnregisteredObject {};

template <parsing::Type type>
class MathWorld
{
public:

  /// @brief type used when looking up a match object with a name at runtime
  using DynMathObject = to_variant_t<tuple_type_cat_t<std::tuple<UnregisteredObject>, tuple_transform_t<ptr, MathObjects<type>>>>;

  /// @brief const version of the one above
  using ConstDynMathObject = to_variant_t<tuple_type_cat_t<std::tuple<UnregisteredObject>, tuple_transform_t<cst_ptr, MathObjects<type>>>>;

  /// @brief default constructor that defines the usual functions and global constants
  MathWorld();

  template <class ObjectType1, size_t size1, class... ObjectTypeN, size_t... sizeN>
  MathWorld(const std::array<std::pair<std::string_view, ObjectType1>, size1>& objects1,
             const std::array<std::pair<std::string_view, ObjectTypeN>, sizeN>&... objectsN);

  template <class ObjectType, size_t size>
  MathWorld(const std::array<std::pair<std::string_view, ObjectType>, size>& objects);

  /// @brief get object from name, the underlying type is to be dynamically resolved at runtime
  /// @note const version
  ConstDynMathObject get(std::string_view name) const;

  /// @brief get object from name, the underlying type is to be dynamically resolved at runtime
  DynMathObject get(std::string_view name);

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

  /// @brief default constructs an ObjectType in the world, under the name 'name'
  ///        then, if there are extra args, forwards them the member function .set() of the newly added object
  /// @param arg...: arguments passed to the set() member function of the object
  /// @note returns a NameError if the name is already taken or has the wrong format, leaves the world unchanged.
  template <class ObjectType, class... Arg>
    requires(tuple_contains_v<MathObjects<type>, ObjectType>
             and (sizeof...(Arg) == 0 or requires(ObjectType o) { o.set(std::declval<Arg>()...); }))
  tl::expected<ref<ObjectType>, NameError> add(std::string_view name, Arg&&... arg);

  /// @brief says if an object with the given name exists within the world
  bool contains(std::string_view name) const;

  /// @brief evaluates a given expression within this world
  tl::expected<double, Error> evaluate(std::string expr) const;

  /// @brief maximum recursion depth to reach before returning an error
  size_t max_recursion_depth = 20;


protected:

  /// @brief converts a DynMathObject to a ConstDynMathObject
  ConstDynMathObject to_const(DynMathObject obj) const;

  /// @brief maps an object name to its type and ID (index within the container that holds it)
  name_map<DynMathObject> inventory;

  tuple_transform_t<SlottedDeque, MathObjects<type>> math_objects;

};

}
