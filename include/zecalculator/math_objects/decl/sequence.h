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

#include "zecalculator/parsing/parser.h"
#include <zecalculator/math_objects/decl/function.h>
#include <zecalculator/mathworld/mathworld.h>

/* TODO: update approach as the following:
   - Check for validity
   - Enable setting custom names for functions and variables
   - Performance improvement: flatten trees
*/

namespace zc {

namespace eval{
namespace ast{
  struct Function;
}
namespace rpn{
  struct Function;
}
}

template <parsing::Type>
class Sequence;

namespace ast {
  using Sequence = zc::Sequence<parsing::Type::AST>;
}

namespace rpn {
  using Sequence = zc::Sequence<parsing::Type::RPN>;
}

/// @brief a class that represents a Sequence of single argument
template <parsing::Type type>
class Sequence: public zc::Function<type>
{
public:
  explicit Sequence() = default;

  explicit Sequence(std::string var_name,
                    const std::string& expr,
                    std::vector<double> first_vals = {});

  /// \brief set the expression
  void set_expression(const std::string& expr);

  void set_input_var(std::string var_name);

  void set_first_values(std::vector<double> first_vals);

  void set_first_val_index(int index);

  constexpr int get_first_val_index() const;

  /// @brief evaluates the sequence at the given index
  /// @note evaluation modifies the state of the sequence, as values get saved within
  ///       the instance, and a locking mechanism is triggered to detect ill-formed seqs
  tl::expected<double, eval::Error> evaluate(double index, const MathWorld<type>& world) const;

  /// @brief operator version of evaluate
  tl::expected<double, eval::Error> operator () (double index, const MathWorld<type>& world) const;

protected:

  /// @brief evaluation with recursion depth tracking
  tl::expected<double, eval::Error> evaluate(double index,
                                             const MathWorld<type>& world,
                                             size_t current_recursion_depth) const;

  friend tl::expected<double, eval::Error> evaluate(const ast::Tree& tree,
                                                    const name_map<double>& input_vars,
                                                    const ast::MathWorld& world,
                                                    size_t current_recursion_depth);

  friend tl::expected<double, eval::Error> evaluate(const rpn::RPN& rpn_expr,
                                                    const name_map<double>& input_vars,
                                                    const rpn::MathWorld& world,
                                                    size_t current_recursion_depth);

  friend struct eval::ast::Function;
  friend struct eval::rpn::Function;

  // hide functions that are not needed from Function
  using Function<type>::evaluate;
  using Function<type>::set_input_vars;

  // index of the first value
  int first_val_index = 0;

  // first values of the sequence
  std::vector<double> values;

};

}
