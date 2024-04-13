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

#pragma once

#include <string>
#include <optional>
#include <span>

#include <zecalculator/error.h>
#include <zecalculator/external/expected.h>
#include <zecalculator/math_objects/decl/math_eq_object.h>
#include <zecalculator/mathworld/decl/mathworld.h>
#include <zecalculator/parsing/data_structures/decl/fast.h>
#include <zecalculator/parsing/data_structures/decl/rpn.h>
#include <zecalculator/parsing/data_structures/decl/uast.h>
#include <zecalculator/parsing/data_structures/decl/utils.h>
#include <zecalculator/parsing/decl/parser.h>
#include <zecalculator/parsing/types.h>
#include <zecalculator/utils/name_map.h>

/* TODO: update approach as the following:
   - Check for validity
   - Enable setting custom names for functions and variables
   - Performance improvement: flatten trees
*/

namespace zc {

template <parsing::Type type>
class MathWorld;

namespace eval {
namespace rpn {
  template <size_t>
  struct Evaluator;
}

namespace fast {
  template <size_t>
  struct Evaluator;
}
}

namespace parsing {
  struct RpnMaker;
}

template <class T>
struct is_function: std::false_type {};

template <class T>
inline constexpr bool is_function_v = is_function<T>::value;

/// @brief class representing a function
/// @tparam type: representation type (AST, RPN)
/// @tparam args_num: number of arguments the function receives
template <parsing::Type type, size_t args_num>
class Function: public MathEqObject<type>
{
public:

  Function(Function&& f) = default;
  Function& operator = (Function&& f) = default;

  /// @brief returns the number of input variables, if they are valid
  static constexpr size_t argument_size() { return args_num; };

  /// @brief gives the Functions and Variables this function directly depends on
  /// @note  uses only the function's expression (no name lookup is done in
  ///        the MathWorld the function belongs to)
  /// @note  undefined functions & variables in the math world will still be listed
  const std::unordered_map<std::string, deps::ObjectType>& direct_dependencies() const;

  /// @brief gives all the Functions and Variables this function (recursively) depends on
  /// @note  uses only the function's expression (no name lookup is done in
  ///        the MathWorld the function belongs to)
  /// @note  undefined functions & variables in the math world will still be listed
  std::unordered_map<std::string, deps::ObjectType> dependencies() const;

  /// @brief tests if the function is valid, i.e. has a valid expression and input vars
  operator bool () const;

  /// @brief returns the parsing error, if there is any
  std::optional<Error> error() const;

  /// @brief evaluation on a given math world with the given input
  tl::expected<double, Error> evaluate(const std::array<double, args_num>& args) const
    requires (args_num >= 1);

  /// @brief evaluation on a given math world with the given input
  /// @note operator style
  tl::expected<double, Error> operator()(const std::array<double, args_num>& args) const
    requires (args_num >= 1);

  // span version
  tl::expected<double, Error> evaluate(std::span<const double, args_num> args) const
    requires (args_num >= 1);

  tl::expected<double, Error> operator()(std::span<const double, args_num> args) const
    requires (args_num >= 1);

  /// @brief evaluation on a given math world with the given input
  tl::expected<double, Error> evaluate() const
    requires (args_num == 0);

  tl::expected<double, Error> operator()() const
    requires (args_num == 0);


protected:

  // constructor reserved for MathWorld when using add() function
  Function(MathEqObject<type> base,
           std::array<parsing::tokens::Text, args_num> vars);

  /// @brief rebind math object names to actual objects in the math world
  /// @note this function is called when function names changed etc...
  void rebind();

  /// @note version that tracks the current recursion depth
  tl::expected<double, Error> evaluate(std::span<const double, args_num> args,
                                       size_t current_recursion_depth) const;

  /// @brief variable names, as views on the function's 'm_definition' (part of parent MathObject class)
  std::array<parsing::tokens::Text, args_num> vars;

  /// @brief object names this function directly depends on
  std::unordered_map<std::string, deps::ObjectType> direct_deps;

  /// @brief binding of the UAST 'left_expr' (parent MathObject class) to 'mathWorld'
  tl::expected<parsing::Parsing<type>, Error> bound_rhs = tl::unexpected(Error::empty_expression());

  template <size_t>
  friend struct eval::rpn::Evaluator;

  template <size_t>
  friend struct eval::fast::Evaluator;

  friend struct parsing::RpnMaker;

  template <parsing::Type>
  friend class MathWorld;
};

template <parsing::Type type, size_t args_num>
struct is_function<Function<type, args_num>>: std::true_type {};

}
