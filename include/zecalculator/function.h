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

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <iostream>

#include <zecalculator/external/expected.h>
#include <zecalculator/utils/evaluation_error.h>
#include <zecalculator/utils/name_map.h>
#include <zecalculator/utils/syntax_tree.h>
#include <zecalculator/builtin_unary_functions.h>
#include <zecalculator/builtin_binary_functions.h>
#include <zecalculator/global_constant.h>

/* TODO: update approach as the following:
   - Check for validity
   - Enable setting custom names for functions and variables
   - Performance improvement: flatten trees
*/

namespace zc {

class Function;
class Expression;
class Sequence;

using GlobalVariable = Expression;

template <class... MathObjectType>
class MathWorldT;

using MathWorld = MathWorldT<CppUnaryFunction, CppBinaryFunction, GlobalConstant, Function, GlobalVariable, Sequence>;

class Function
{
public:

  struct InvalidInputVar
  {
    std::string var_name;
  };

  explicit Function() = default;

  /// @brief constructor for a function that takes many input variables
  explicit Function(std::vector<std::string> input_vars, const std::string& expr)
  {
    set_input_vars(input_vars);
    set_expression(expr);
  }

  Function(const Function& f)
    : vars(f.vars)
  {
    if (f.expression)
      set_expression(*f.expression);
  }

  Function& operator = (const Function& f)
  {
    vars = f.vars;
    if (f.expression)
      set_expression(*f.expression);
    return *this;
  }

  Function(Function&& f) = default;
  Function& operator = (Function&& f) = default;

  /// @brief sets the names of the input variables
  /// @note the order of the input variables is important when calling the function
  ///       with positional arguments
  void set_input_vars(std::vector<std::string> input_vars)
  {
    auto it = std::ranges::find_if_not(input_vars, is_valid_name);
    if (it != input_vars.end())
      vars = tl::unexpected(InvalidInputVar{*it});
    else vars = std::move(input_vars);
  }

  /// \brief set the expression
  void set_expression(const std::string& expr)
  {
    // do nothing if it's the same expression
    if (expression and *expression == expr)
      return;

    expression = std::make_unique<std::string>(expr);

    // workaround limitation in tl::expected when using and_then to implicitly converted-to types
    auto parsing = parse(*expression);
    if (parsing)
      tree = make_tree(parsing.value());
    else tree = tl::unexpected(parsing.error());
  }

  /// @brief returns the number of input variables, if they are valid
  std::optional<size_t> argument_size() const
  {
    if (vars)
      return vars->size();
    else return {};
  }

  /// @brief tests if the function is valid, i.e. has a valid expression and input vars
  operator bool () const
  {
    return bool(tree) and (not std::holds_alternative<std::monostate>(tree.value())) and bool(vars);
  }

  const tl::expected<SyntaxTree, ParsingError>& get_tree() const { return tree; }

  /// @brief evaluation on a given math world with the given input
  tl::expected<double, EvaluationError> evaluate(const std::vector<double>& args,
                                                 const MathWorld& world) const;

  /// @brief evaluation on a given math world with the given input
  /// @note operator style
  tl::expected<double, EvaluationError> operator () (const std::vector<double>& args,
                                                     const MathWorld& world) const;

protected:

  /// @brief evaluation on a given math world with the given input
  /// @note version that tracks the current recursion depth
  tl::expected<double, EvaluationError> evaluate(const std::vector<double>& args,
                                                 const MathWorld& world,
                                                 size_t current_recursion_depth) const;

  friend tl::expected<double, EvaluationError> evaluate(const SyntaxTree& tree,
                                                        const name_map<double>& input_vars,
                                                        const MathWorld& world,
                                                        size_t current_recursion_depth);

  friend class Sequence;

  std::unique_ptr<std::string> expression;
  // std::unique_ptr because the tree keeps string_views to it
  // and to be able to do move semantics without re-making a tree

  tl::expected<SyntaxTree, ParsingError> tree;
  tl::expected<std::vector<std::string>, InvalidInputVar> vars;

};

}
