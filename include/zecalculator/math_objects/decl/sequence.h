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

#include <zecalculator/math_objects/decl/function.h>

/* TODO: update approach as the following:
   - Check for validity
   - Enable setting custom names for functions and variables
   - Performance improvement: flatten trees
*/

namespace zc {

template <parsing::Type type>
class MathWorld;

template <parsing::Type type>
class DynMathObject;

namespace internal {
  struct EqObject;
}

template <class T>
struct is_sequence: std::false_type {};

template <class T>
inline constexpr bool is_sequence_v = is_sequence<T>::value;

/// @brief a class that represents a Sequence of single argument
template <parsing::Type type>
class Sequence
{
public:

  std::string_view get_name() const { return name; };

  const std::string& get_equation() const { return equation; }

  /// @returns the name of the input variable, as defined in the sequence's equation/definition
  /// @note this name isn't used internally and is replaced an integer index
  const std::string get_input_var_name() const { return input_var_name; }

  /// @brief evaluates the sequence at the given index
  tl::expected<double, Error> evaluate(double index, eval::Cache* cache = nullptr) const;

  /// @brief operator version of evaluate
  tl::expected<double, Error> operator () (double index, eval::Cache* cache = nullptr) const;

protected:

  /// @brief Makes a valid but "empty" Sequence, will evaluate to NaN for any input
  Sequence() = default;

  Sequence(std::string name,
           std::string input_var_name,
           std::string equation,
           std::vector<parsing::Parsing<type>> values);

  /// @brief evaluation with recursion depth tracking
  tl::expected<double, Error> evaluate(double index,
                                       size_t current_recursion_depth,
                                       eval::Cache* cache = nullptr) const;

  template <parsing::Type>
  friend struct eval::Evaluator;

  std::string name;

  std::string input_var_name;

  std::string equation;

  /// @brief first values of the sequence
  /// @note the last value is the "default" expression
  /// @example Fibonacci: values = {parsing_of("1"), parsing_of("1"), parsing_of("u(n-1) + u(n-2)")}
  std::vector<parsing::Parsing<type>> values;

  friend struct internal::EqObject;

};

template <parsing::Type type>
struct is_sequence<Sequence<type>>: std::true_type {};

template <parsing::Type type>
struct is_function<Sequence<type>>: std::true_type {};

}
