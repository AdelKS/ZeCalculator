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
#include <vector>
#include <optional>
#include <span>

#include <zecalculator/error.h>
#include <zecalculator/external/expected.h>
#include <zecalculator/parsing/data_structures/decl/node.h>
#include <zecalculator/parsing/shared.h>
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

namespace ast {
  template <size_t>
  struct Evaluator;
}
}

namespace parsing {
  struct RpnMaker;
}

namespace deps {
  /// @brief used to know the type of the dependency when querying deps
  enum ObjectType {VARIABLE, FUNCTION};
}

template <size_t args_num>
using Vars = std::array<std::string, args_num>;

/// @brief class that handles Function's input vars
template <size_t args_num>
struct InputVars
{
  tl::expected<std::array<std::string, args_num>, Error> vars;
};

template <>
struct InputVars<0>
{};

template <parsing::Type type, size_t args_num>
class Function: public InputVars<args_num>
{
public:

  Function(Function&& f) = default;
  Function& operator = (Function&& f) = default;

  const std::string& get_name() const;

  /// @brief sets the names of the input variables
  /// @note the order of the input variables is important when calling the function
  ///       with positional arguments
  void set_input_vars(Vars<args_num> input_vars)
    requires (args_num > 0);

  void set_input_var(std::string input_var)
    requires (args_num == 1);

  /// \brief set the expression
  void set_expression(std::string expr);

  /// @brief sets both the input_vars and the expression
  void set(Vars<args_num> input_vars, std::string expr)
    requires (args_num >= 1);

  void set(std::string expr)
    requires (args_num == 0);

  /// @brief returns the number of input variables, if they are valid
  static constexpr size_t argument_size() { return args_num; };

  /// @brief gives the Functions and Variables this function directly depends on
  /// @note  uses only the function's expression (no name lookup is done in
  ///        the MathWorld the function belongs to)
  /// @note  undefined functions & variables in the math world will still be listed
  std::unordered_map<std::string, deps::ObjectType> direct_dependencies() const;

  /// @brief tests if the function is valid, i.e. has a valid expression and input vars
  operator bool () const;

  /// @brief returns the parsing error, if there is any
  std::optional<Error> error() const;

  const tl::expected<parsing::Parsing<type>, Error>& get_parsing() const;

  /// @brief (re)parse the expression
  void parse();

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
  Function(const MathWorld<type>*);

  /// @note version that tracks the current recursion depth
  tl::expected<double, Error> evaluate(std::span<const double, args_num> args,
                                       size_t current_recursion_depth) const;

  void set_name(std::string name);

  std::string name;
  std::string expression;

  template <size_t>
  friend struct eval::rpn::Evaluator;

  template <size_t>
  friend struct eval::ast::Evaluator;

  friend struct parsing::RpnMaker;

  tl::expected<std::vector<parsing::Token>, Error> tokenized_expr;
  tl::expected<parsing::Parsing<type>, Error> parsed_expr;
  tl::expected<Vars<args_num>, Error> vars;

  // non-owning pointer to the mathworld that contains this object
  const MathWorld<type>* mathworld;

  template <parsing::Type>
  friend class MathWorld;
};

}
