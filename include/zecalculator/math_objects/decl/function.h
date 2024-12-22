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

#include <span>

#include <zecalculator/error.h>
#include <zecalculator/external/expected.h>
#include <zecalculator/mathworld/decl/mathworld.h>
#include <zecalculator/parsing/data_structures/decl/ast.h>
#include <zecalculator/parsing/data_structures/decl/fast.h>
#include <zecalculator/parsing/data_structures/decl/rpn.h>
#include <zecalculator/parsing/data_structures/decl/utils.h>
#include <zecalculator/parsing/data_structures/deps.h>
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

template <parsing::Type type>
class DynMathObject;

namespace eval {
  template <parsing::Type>
  struct Evaluator;
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
template <parsing::Type type>
class Function: public MathObject
{
public:

  /// @brief returns the number of input variables, if they are valid
  size_t args_num() const { return argument_number; };

  const std::string get_equation() const { return equation; }

  tl::expected<double, Error> evaluate(std::initializer_list<double> vals = {}, eval::Cache* cache = nullptr) const;
  tl::expected<double, Error> operator()(std::initializer_list<double> vals = {}, eval::Cache* cache = nullptr) const;

protected:

  /// @details used as an intermediary state when "linking" functions
  /// @note    this state should never be exposed to downstream users of the library
  Function(size_t argument_number): argument_number(argument_number) {};

  Function(MathObject base,
           std::string equation,
           size_t argument_number,
           parsing::Parsing<type> parsing);

  /// @brief binding of the AST 'left_expr' (parent MathObject class) to 'mathWorld'
  std::optional<parsing::Parsing<type>> bound_rhs = {};

  std::string equation = {};

  size_t argument_number;

  template <parsing::Type>
  friend struct eval::Evaluator;

  friend struct parsing::RpnMaker;

  friend struct EqObject;
};

template <parsing::Type type>
struct is_function<Function<type>>: std::true_type {};

}
