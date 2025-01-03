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
#include <zecalculator/evaluation/decl/cache.h>
#include <zecalculator/external/expected.h>
#include <zecalculator/math_objects/object_list.h>
#include <zecalculator/parsing/data_structures/deps.h>
#include <zecalculator/parsing/decl/utils.h>
#include <zecalculator/parsing/types.h>

namespace zc {

template <parsing::Type type>
class MathWorld;

namespace parsing {
  template <Type type>
  struct make_fast;
}

template <parsing::Type type>
class DynMathObject: public tl::expected<MathObjectsVariant<type>, Error>
{
public:
  /// @brief assign equation and automatically deduce the object type the equation defines
  /// @note this method can potentially modify every other DynMathObject in the same MathWorld
  DynMathObject<type>& operator = (std::string eq);

  /// @brief assign equation and automatically deduce the object type the equation defines
  /// @note this method can potentially modify every other DynMathObject in the same MathWorld
  template <size_t args_num>
  DynMathObject<type>& set(std::string name, CppFunction<args_num> cpp_f);

  /// @note this method can potentially modify every other DynMathObject in the same MathWorld
  DynMathObject<type>& set(std::string name, GlobalConstant cst);

  /// @note this method can potentially modify every other DynMathObject in the same MathWorld
  DynMathObject<type>& set_data(std::string name, std::vector<std::string> data);

  tl::expected<double, Error> operator () (std::initializer_list<double> vals = {}, eval::Cache* cache = nullptr) const;
  tl::expected<double, Error> evaluate(std::initializer_list<double> vals = {}, eval::Cache* cache = nullptr) const;

  /// @brief returns the currently set name, regardless of the validity of the object
  /// @note returns non-empty string only if the object has been assigned a valid unique name
  std::string_view get_name() const;

  /// @brief returns the names of the variables that are used as input to the function
  /// @note  empty if there are not
  std::vector<std::string> get_input_var_names() const;

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

  /// @brief gets the equation assigned to the object, if there is one
  std::optional<std::string> get_equation() const;

protected:
  const size_t slot;
  MathWorld<type>& mathworld;

  enum Type {BAD_EQUATION, CONSTANT, CPP_FUNCTION, FUNCTION, SEQUENCE, DATA};
  Type obj_type = BAD_EQUATION;

  std::optional<std::string> opt_equation;

  struct FuncObj {
    parsing::AST rhs;
  };

  struct SeqObj {
    std::vector<parsing::AST> rhs;
  };

  struct DataObj {
    std::vector<std::string> data;
  };

  std::variant<zc::Error, double, FuncObj, SeqObj, DataObj, CppFunction<1>, CppFunction<2>> parsed_data = zc::Error::empty_expression();

  tl::expected<parsing::LHS, zc::Error> exp_lhs = tl::unexpected(zc::Error::empty_expression());

  DynMathObject(tl::expected<MathObjectsVariant<type>, Error> exp_variant, size_t slot, MathWorld<type>& mathworld);

  /// @tparam linked: link with other math objects, otherwise assigns unlinked alternative
  template <bool linked = true>
  DynMathObject<type>& assign_alternative();

  friend MathWorld<type>;
};

} // namespace zc
