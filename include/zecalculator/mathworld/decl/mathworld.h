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
#include <zecalculator/math_objects/decl/dyn_math_object.h>
#include <zecalculator/math_objects/object_list.h>
#include <zecalculator/parsing/data_structures/ast.h>
#include <zecalculator/parsing/data_structures/deps.h>
#include <zecalculator/utils/name_map.h>
#include <zecalculator/utils/refs.h>
#include <zecalculator/utils/slotted_deque.h>
#include <zecalculator/utils/tuple.h>
#include <zecalculator/utils/utils.h>

#include <string>
#include <string_view>
#include <unordered_set>

namespace zc {

template <parsing::Type type>
class MathWorld;

namespace parsing {
  template <Type type>
  struct make_fast;
}

namespace fast {
  using MathWorld = zc::MathWorld<parsing::Type::FAST>;
}

namespace rpn {
  using MathWorld = zc::MathWorld<parsing::Type::RPN>;
}

template <parsing::Type type>
class MathWorld
{
public:

  using iterator = SlottedDeque<DynMathObject<type>>::iterator;
  using const_iterator = SlottedDeque<DynMathObject<type>>::const_iterator;

  /// @brief default constructor that defines the usual functions and global constants
  MathWorld();

  iterator begin();
  const_iterator begin() const;
  const_iterator cbegin() const;
  iterator end();
  const_iterator end() const;
  const_iterator cend() const;

  /// @brief get DynMathObject from 'slot'
  /// @note returns nullptr if 'slot' is out of bounds or unassigned
  DynMathObject<type>* get(size_t slot);

  /// @note const version
  const DynMathObject<type>* get(size_t slot) const;

  /// @brief get DynMathObject from name
  /// @note returns nullptr if 'name' does not refer to any known object
  DynMathObject<type>* get(std::string_view name);

  /// @note const version
  const DynMathObject<type>* get(std::string_view name) const;

  /// @brief returns a handle to a new math object
  /// @note  the expected within is in error state
  DynMathObject<type>& new_object();

  /// @brief says if an object with the given name exists within the world
  bool contains(std::string_view name) const;

  /// @brief gives the Functions and Variables the equation of this object directly depends on
  /// @note  uses only the equation of objects defined through an equation
  ///        -> undefined functions & variables in the math world will still be listed
  /// @note  return an empty container for objects not defined with an equation
  deps::Deps direct_dependencies(std::string_view name) const;

  /// @brief evaluates a given expression within this world
  tl::expected<double, Error> evaluate(std::string expr) const;

  /// @brief return the direct reverse dependencies, aka objects that depend directly on 'name'
  /// @note  this function is non-const because dependencies are cached and this function may trigger caching
  deps::Deps direct_revdeps(std::string_view name);

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

protected:

  /// @brief object at 'slot' changed name, became invalid / deleted, or got a new name
  /// @note 'old_name' may be empty, in which case it's a new name
  /// @note 'new_name' may be empty, in which case the object got deleted or is in an invalid state
  void object_updated(size_t slot,
                      std::string old_name,
                      std::string new_name);

  /// @brief checks that this object has actually been allocated in this world
  bool sanity_check(const DynMathObject<type>& obj) const;

  /// @brief return all the DynMathObjects (not necessarily in a valid state) that have an EqObject depend on 'names'
  std::unordered_set<DynMathObject<type>*>
    dependent_eq_objects(const std::unordered_set<std::string>& names);

  /// @brief go through all functions that depend on 'old_name' or 'new_name' and rebind them
  void rebind_dependent_functions(const std::unordered_set<std::string>& names);

  /// @brief maps an object name to its slot
  name_map<size_t> inventory;

  SlottedDeque<DynMathObject<type>> math_objects;

  friend DynMathObject<type>;

  template <parsing::Type>
  friend struct parsing::make_fast;

};

}
