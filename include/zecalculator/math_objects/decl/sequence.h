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

using Vals = std::vector<double>;

template <class T>
struct is_sequence: std::false_type {};

template <class T>
inline constexpr bool is_sequence_v = is_sequence<T>::value;

/// @brief a class that represents a Sequence of single argument
template <parsing::Type type>
class Sequence: public zc::Function<type, 1>
{
  using Parent = Function<type, 1>;
public:

  void set(std::string var_name,
           const std::string& expr,
           std::vector<double> first_vals = {});

  void set_input_var(std::string var_name);

  void set_first_values(std::vector<double> first_vals);

  void set_first_val_index(int index);

  constexpr int get_first_val_index() const;

  /// @brief evaluates the sequence at the given index
  /// @note evaluation modifies the state of the sequence, as values get saved within
  ///       the instance, and a locking mechanism is triggered to detect ill-formed seqs
  tl::expected<double, Error> evaluate(double index) const;

  /// @brief operator version of evaluate
  tl::expected<double, Error> operator () (double index) const;

protected:

  // constructor reserved for MathWorld when using add() function
  Sequence(MathWorldObjectHandle<type> obj_handle);

  /// @brief evaluation with recursion depth tracking
  tl::expected<double, Error> evaluate(double index, size_t current_recursion_depth) const;

  template <size_t>
  friend struct eval::rpn::Evaluator;

  template <size_t>
  friend struct eval::ast::Evaluator;

  // hide functions that are not needed from Function
  using Parent::evaluate;
  using Parent::operator();

  // index of the first value
  int first_val_index = 0;

  // first values of the sequence
  std::vector<double> values;

  template <parsing::Type>
  friend class MathWorld;

};

template <parsing::Type type>
struct is_sequence<Sequence<type>>: std::true_type {};

template <parsing::Type type>
struct is_function<Sequence<type>>: std::true_type {};

}
