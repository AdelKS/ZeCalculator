/****************************************************************************
**  Copyright (c) 2023, Adel Kara Slimane <adel.ks@zegrapher.com>
**
**  This file is part of ZeGrapher's source code.
**
**  ZeGrapher is free software: you may copy, redistribute and/or modify it
**  under the terms of the GNU General Public License as published by the
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

#include <zecalculator/function.h>

/* TODO: update approach as the following:
   - Check for validity
   - Enable setting custom names for functions and variables
   - Performance improvement: flatten trees
*/

namespace zc {

/// @brief a class that represents a Sequence of single argument
class Sequence: public Function
{
public:
  explicit Sequence() = default;

  explicit Sequence(std::string var_name,
                    const std::string& expr,
                    std::vector<double> first_vals = {})
    : Function(std::vector{var_name}, expr), values(first_vals)
  {}

  /// \brief set the expression
  void set_expression(const std::string& expr)
  {
    Function::set_expression(expr);
    values.clear();
  }

  void set_input_var(std::string var_name)
  {
    set_input_vars(std::vector {var_name});
  }

  void set_first_values(std::vector<double> first_vals)
  {
    values = std::move(first_vals);
  }

  void set_first_val_index(int index)
  {
    first_val_index = index;
    values.clear();
  }

  constexpr int get_first_val_index() { return first_val_index; };

  /// @brief evaluates the sequence at the given index
  /// @note evaluation modifies the state of the sequence, as values get saved within
  ///       the instance, and a locking mechanism is triggered to detect ill-formed seqs
  tl::expected<double, EvaluationError> evaluate(double index, const MathWorld& world) const;

  /// @brief operator version of evaluate
  tl::expected<double, EvaluationError> operator () (double index, const MathWorld& world) const;

protected:

  /// @brief evaluation with recursion depth tracking
  tl::expected<double, EvaluationError> evaluate(double index,
                                                 const MathWorld& world,
                                                 size_t current_recursion_depth) const;

  friend tl::expected<double, EvaluationError> evaluate(const SyntaxTree& tree,
                                                        const name_map<double>& input_vars,
                                                        const MathWorld& world,
                                                        size_t current_recursion_depth);

  // hide functions that are not needed from Function
  using Function::evaluate;
  using Function::set_input_vars;

  // index of the first value
  int first_val_index = 0;

  // first values of the sequence
  std::vector<double> values;

};

}
