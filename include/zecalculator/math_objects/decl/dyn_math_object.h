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
#include <zecalculator/external/expected.h>
#include <zecalculator/math_objects/decl/eq_object.h>
#include <zecalculator/math_objects/decl/math_object.h>
#include <zecalculator/math_objects/object_list.h>
#include <zecalculator/parsing/data_structures/deps.h>
#include <zecalculator/parsing/types.h>

namespace zc {

template <parsing::Type type>
class MathWorld;

namespace parsing {
  template <Type type>
  struct make_fast;
}

template <class T>
struct As { std::string str; };

template <parsing::Type type>
class DynMathObject: public tl::expected<MathObjectsVariant<type>, Error>
{
public:
  /// @brief assign equation and interpret as defining a object of type 'T'
  /// @note even if the equation is syntactically correct, if it cannot define an object of type 'T'
  ///       this instance will hold an error
  /// @note this method can potentially modify every other DynMathObject in the same MathWorld
  template <class T>
    requires tuple_contains_v<MathEqObjects<type>, T>
  DynMathObject<type>& operator = (As<T> eq);

  /// @brief assign equation and automatically deduce the object type the equation defines
  /// @note this method can potentially modify every other DynMathObject in the same MathWorld
  DynMathObject<type>& operator = (std::string eq);

  /// @brief assign equation and automatically deduce the object type the equation defines
  /// @note this method can potentially modify every other DynMathObject in the same MathWorld
  template <size_t args_num>
  DynMathObject<type>& operator = (CppFunction<args_num> cpp_f);

  /// @note this method can potentially modify every other DynMathObject in the same MathWorld
  DynMathObject<type>& operator = (GlobalConstant cst);

  template <class... DBL>
    requires (std::is_convertible_v<DBL, double> and ...)
  tl::expected<double, Error> operator () (DBL... val) const;

  template <class... DBL>
    requires (std::is_convertible_v<DBL, double> and ...)
  tl::expected<double, Error> evaluate(DBL... val) const;

  /// @brief returns the name of the object, if it's valid, otherwise empty string
  std::string_view get_name() const;

  /// @brief gets the value of the expected (the variant) as a specific alternative
  template <class T>
    requires (tuple_contains_v<MathObjects<type>, T>)
  const T& value_as() const;

  /// @brief gets the value of the expected (the variant) as a specific alternative
  template <class T>
    requires (tuple_contains_v<MathObjects<type>, T>)
  T& value_as();

  template <class T>
    requires (tuple_contains_v<MathObjects<type>, T> or std::is_same_v<T, Error>)
  bool holds() const;

  /// @brief gives the Functions and Variables the equation of this object directly depends on
  /// @note  uses only the equation this object is defined with
  ///        -> undefined functions & variables in the math world will still be listed
  /// @note  return an empty container when not defined through an equation
  deps::Deps direct_dependencies() const;

  /// @brief no pun intended
  tl::expected<MathObjectsVariant<type>, Error>& as_expected();

  /// @brief no pun intended
  const tl::expected<MathObjectsVariant<type>, Error>& as_expected() const;

  /// @brief returns object's slot within its owning MathWorld
  size_t get_slot() const { return slot; }

  /// @brief gets the expression assigned to the object, if there is one
  /// @note  returns an expression even in error state, if due to an erroneous expression
  std::optional<std::string> get_equation() const;

protected:
  const size_t slot;
  MathWorld<type>& mathworld;

  /// @brief non-empty when a syntactically correct equation gets assigned
  std::optional<EqObject> opt_eq_object;

  DynMathObject(tl::expected<MathObjectsVariant<type>, Error> exp_variant, size_t slot, MathWorld<type>& mathworld);

  DynMathObject<type>& assign(std::string definition, EqObject::Category cat);

  DynMathObject<type>& assign_error(Error error, std::optional<EqObject> new_opt_eq_obj = {});

  template <class T>
  DynMathObject<type>& assign_object(T&& obj, std::optional<EqObject> new_opt_eq_obj);

  /// @brief true if has an assigned 'opt_eq_object' with a function/sequence within
  bool has_function_eq_obj() const;

  friend MathWorld<type>;
};

} // namespace zc
