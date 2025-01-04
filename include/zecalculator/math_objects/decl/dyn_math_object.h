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
#include <zecalculator/parsing/data_structures/decl/utils.h>
#include <zecalculator/parsing/data_structures/deps.h>
#include <zecalculator/parsing/decl/utils.h>
#include <zecalculator/parsing/types.h>

namespace zc {

template <parsing::Type type>
class MathWorld;

namespace parsing {
  template <Type>
  struct make_fast;

  template <parsing::Type>
  struct FunctionVisiter;

  template <parsing::Type>
  struct VariableVisiter;
}

enum ObjectType {BAD_EQUATION, CONSTANT, CPP_FUNCTION, FUNCTION, SEQUENCE, DATA};

template <parsing::Type type>
class DynMathObject
{
public:
  /// @brief assign equation and automatically deduce the object type the equation defines
  /// @note this method can potentially modify every other DynMathObject in the same MathWorld
  DynMathObject<type>& operator = (std::string eq);

  /// @brief changes the name of the object
  /// @note if the name is already taken, then the name becomes free, user needs to call this function again
  /// @note does not update the name within the equation string, if there is one. Errors will may have a wrong offset because of that
  DynMathObject<type>& set_name(std::string_view name);

  /// @brief assign equation and automatically deduce the object type the equation defines
  /// @note 'name' should not be a function call
  /// @note this method can potentially modify every other DynMathObject in the same MathWorld
  template <size_t args_num>
  DynMathObject<type>& set(std::string_view name, CppFunction<args_num> cpp_f);

  template <size_t args_num>
  DynMathObject<type>& operator = (CppFunction<args_num> cpp_f);

  /// @brief define the object as a global constant of value 'value'
  /// @note 'name' should not be a function call
  /// @note this method can potentially modify every other DynMathObject in the same MathWorld
  DynMathObject<type>& set(std::string_view name, double value);
  DynMathObject<type>& operator = (double value);

  /// @note this method can potentially modify every other DynMathObject in the same MathWorld
  DynMathObject<type>& set_data(std::string_view name, std::vector<std::string> data);
  DynMathObject<type>& set_data_point(size_t index, std::string expr);

  tl::expected<double, Error> operator () (std::initializer_list<double> vals = {}, eval::Cache* cache = nullptr) const;
  tl::expected<double, Error> evaluate(std::initializer_list<double> vals = {}, eval::Cache* cache = nullptr) const;

  /// @brief returns the currently set name, regardless of the validity of the object
  /// @note returns non-empty string only if the object has been assigned a valid unique name
  std::string_view get_name() const;

  /// @brief returns the names of the variables that are used as input to the function
  /// @note  empty if there are not
  std::vector<std::string> get_input_var_names() const;

  /// @returns the type of the object held by this instance
  ObjectType object_type() const;

  /// @returns true if the instance is currently of that type, regardless of its validity
  bool holds(ObjectType obj_type) const;

  /// @brief returns true if name_status() and object_status() are okay;
  operator bool () const;

  /// @brief same as operator bool()
  bool has_value () const { return bool(*this); }

  /// @brief returns the status of the name / "Left hand side" of the object
  tl::expected<Ok, zc::Error> name_status() const;

  /// @brief returns the status of the object / "right hand side" of the object
  tl::expected<Ok, zc::Error> object_status() const;

  /// @brief returns the overall status of the object, aka rhs and lhs
  tl::expected<Ok, zc::Error> status() const;

  /// @brief returns either the error reported by name_status() or object_status(), if there is one
  std::optional<zc::Error> error() const;

  /// @brief gives the Functions and Variables the equation of this object directly depends on
  /// @note  uses only the equation this object is defined with
  ///        -> undefined functions & variables in the math world will still be listed
  /// @note  return an empty container when not defined through an equation
  deps::Deps direct_dependencies() const;

  /// @brief returns object's slot within its owning MathWorld
  size_t get_slot() const { return slot; }

  /// @brief gets the equation assigned to the object, if there is one
  std::optional<std::string> get_equation() const;

  using LinkedRepr = std::variant<CppFunction<1>,
                                  CppFunction<2>,
                                  const double*,
                                  const parsing::LinkedFunc<type>*,
                                  const parsing::LinkedSeq<type>*,
                                  const parsing::LinkedData<type>*>;

  /// @brief returns the internal linked representation, if there's one
  /// @note  this function is offered for debugging purposes / advanced use
  tl::expected<LinkedRepr, zc::Error> get_linked_repr() const;

protected:
  const size_t slot;
  MathWorld<type>& mathworld;

  struct ConstObj {
    double val;
    std::optional<std::string> rhs_str = {};
  };

  struct FuncObj {
    /// @brief the string also contains the equal sign that acts as a separator
    std::string rhs_str;
    parsing::AST rhs;
    tl::expected<parsing::LinkedFunc<type>, zc::Error> linked_rhs = tl::unexpected(
      zc::Error::empty_expression());
  };

  struct SeqObj {
    std::string rhs_str;
    std::vector<parsing::AST> rhs = {};
    tl::expected<parsing::LinkedSeq<type>, zc::Error> linked_rhs = tl::unexpected(
      zc::Error::empty_expression());
  };

  struct DataObj {
    std::vector<std::string> data = {};
    std::vector<tl::expected<parsing::AST, zc::Error>> rhs = {};
    parsing::LinkedData<type> linked_rhs = {};
  };

  std::variant<zc::Error, ConstObj, FuncObj, SeqObj, DataObj, CppFunction<1>, CppFunction<2>>
    parsed_data = zc::Error::empty_expression();

  std::string lhs_str;
  tl::expected<parsing::LHS, zc::Error> exp_lhs = tl::unexpected(zc::Error::empty_expression());

  DynMathObject(size_t slot, MathWorld<type>& mathworld): slot(slot), mathworld(mathworld) {};

  /// @brief updates the name of the object without notifying the MathWorld instance about it
  template <class T>
    requires (std::is_same_v<T, parsing::AST> or std::is_convertible_v<T, std::string_view>)
  void set_name_internal(const T& name, std::string_view full_expr);

  tl::expected<zc::parsing::Parsing<type>, zc::Error> get_final_repr(const parsing::AST& ast,
                                                                     std::string_view equation);

  /// @tparam linked: link with other math objects, otherwise assigns unlinked alternative
  template <bool link = true>
  DynMathObject<type>& finalize_asts();

  friend MathWorld<type>;

  friend struct parsing::FunctionVisiter<type>;
  friend struct parsing::VariableVisiter<type>;
  friend struct parsing::make_fast<type>;
};

} // namespace zc
