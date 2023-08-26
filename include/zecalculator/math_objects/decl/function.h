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

#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include <optional>
#include <iostream>

#include <zecalculator/evaluation/error.h>
#include <zecalculator/external/expected.h>
#include <zecalculator/math_objects/builtin_binary_functions.h>
#include <zecalculator/math_objects/builtin_unary_functions.h>
#include <zecalculator/math_objects/global_constant.h>
#include <zecalculator/mathworld/decl/mathworld.h>
#include <zecalculator/parsing/parser.h>
#include <zecalculator/utils/name_map.h>

/* TODO: update approach as the following:
   - Check for validity
   - Enable setting custom names for functions and variables
   - Performance improvement: flatten trees
*/

namespace zc {

namespace eval{
  struct Function;
}

template <parsing::Type>
class Function;

namespace ast {
  using Function = zc::Function<parsing::Type::AST>;
}

struct Ok {};
struct Empty {};

struct InvalidInputVar
{
  std::string var_name;
};

template <parsing::Type type>
class Function
{
public:

  explicit Function() = default;

  /// @brief constructor for a function that takes many input variables
  explicit Function(std::vector<std::string> input_vars, std::string expr);

  Function(const Function& f) = default;
  Function(Function&& f) = default;

  Function& operator = (const Function& f) = default;
  Function& operator = (Function&& f) = default;

  /// @brief sets the names of the input variables
  /// @note the order of the input variables is important when calling the function
  ///       with positional arguments
  void set_input_vars(std::vector<std::string> input_vars);

  /// \brief set the expression
  void set_expression(std::string expr);

  /// @brief returns the number of input variables, if they are valid
  std::optional<size_t> argument_size() const;

  /// @brief tests if the function is valid, i.e. has a valid expression and input vars
  operator bool () const;

  std::variant<Ok, Empty, parsing::Error> parsing_status() const;

  const tl::expected<ast::Tree, parsing::Error>& get_tree() const
    requires (type == parsing::AST);

  const tl::expected<rpn::RPN, parsing::Error>& get_rpn() const
    requires (type == parsing::RPN);

  /// @brief evaluation on a given math world with the given input
  tl::expected<double, eval::Error> evaluate(const std::vector<double>& args,
                                             const MathWorld<type>& world) const;

  /// @brief evaluation on a given math world with the given input
  /// @note operator style
  tl::expected<double, eval::Error> operator () (const std::vector<double>& args,
                                                     const MathWorld<type>& world) const;

protected:

  /// @brief evaluation on a given math world with the given input
  /// @note version that tracks the current recursion depth
  tl::expected<double, eval::Error> evaluate(const std::vector<double>& args,
                                             const MathWorld<type>& world,
                                             size_t current_recursion_depth) const;

  friend tl::expected<double, eval::Error> evaluate(const ast::Tree& tree,
                                                    const name_map<double>& input_vars,
                                                    const MathWorld<parsing::AST>& world,
                                                    size_t current_recursion_depth);

  friend struct eval::Function;

  friend class Sequence<type>;

  std::string expression;

  using ParsingType = typename std::conditional_t<type == parsing::AST, ast::Tree, rpn::RPN>;

  tl::expected<ParsingType, parsing::Error> parsed_expr;
  tl::expected<std::vector<std::string>, InvalidInputVar> vars;

};

}
