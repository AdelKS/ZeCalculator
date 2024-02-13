/****************************************************************************
**  Copyright (c) 2023, Adel Kara Slimane <adel.ks@zegrapher.com>
**
**  This file is part of ZeCalculator.
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

#include <zecalculator/error.h>
#include <zecalculator/mathworld/decl/mathworld.h>
#include <zecalculator/parsing/data_structures/decl/ast.h>
#include <zecalculator/parsing/data_structures/decl/rpn.h>
#include <zecalculator/parsing/data_structures/decl/uast.h>
#include <zecalculator/parsing/data_structures/decl/utils.h>
#include <zecalculator/parsing/data_structures/token.h>

#include <span>

/* TODO: update approach as the following:
    - Parse: aka cut each atom in a formula
    - Evaluate type of atom: separator, number
    - Treat number in a special as to make 1.2E+33 as one atom
    - Check for validity
    - Enable setting custom names for functions and variables
    - Performance improvement: flatten trees
*/

namespace zc {

namespace deps {
  /// @brief used to know the type of the dependency when querying deps
  enum ObjectType {VARIABLE, FUNCTION};

  using Deps = std::unordered_map<std::string, ObjectType>;
} // namespace deps

namespace parsing {

/// @brief interprets "view" as a floating number
/// @returns if successful, the interpreted double and the number of characters interpreted, otherwise empty
std::optional<std::pair<double, size_t>> to_double(std::string_view view);

/// @brief parses the expression into a list of tokens
/// @note the string that is void must remain valid for for the returned instance
///       to remain valid (for both a successful or unsuccessful  parsing)
///       as they contain sub-string views of the input view
tl::expected<std::vector<Token>, Error> tokenize(std::string_view expression);

/// @brief tells if the string_view contains a valid math object name
bool is_valid_name(std::string_view name);

/// @brief makes a syntax tree from from a sequence of tokens
/// @param input_vars: variable names that are considered as input (for functions)
///                    e.g."x" in the function "f" such as "f(x) = cos(x)"
template <std::ranges::viewable_range Range = std::array<std::string, 0>>
  requires std::is_convertible_v<std::ranges::range_value_t<Range>, std::string_view>
tl::expected<UAST, Error> make_uast(std::span<const parsing::Token> tokens,
                                    const Range& input_vars = std::array<std::string, 0>{});

template <std::ranges::viewable_range Range>
  requires std::is_convertible_v<std::ranges::range_value_t<Range>, std::string_view>
struct mark_input_vars
{
  const Range& input_vars;

  /// @brief returns a copy of 'tre' where 'uast::node::Variable' instances are replaced
  ///         with uast::node::InputVariable when name is in 'input_vars'
  UAST operator () (const UAST& tree);
};


// user deduction guide for clang-16 that is stupid
template <class T>
mark_input_vars(T) -> mark_input_vars<T>;

/// @brief functor that transforms an UAST to an AST<type> by doing object name lookup within
///        a MathWorld instance and binding to objects with references
template <Type type>
struct bind;

/// @brief transforms a syntax tree to a flat Reverse Polish / postfix notation representation
RPN make_RPN(const AST<Type::RPN>& tree);

/// @brief gives the Function and Variable names in these tokens
///        variable names in `input_vars` will be filter out
template <std::ranges::viewable_range Range = std::array<std::string, 0>>
  requires std::is_convertible_v<std::ranges::range_value_t<Range>, std::string_view>
deps::Deps direct_dependencies(const std::vector<parsing::Token>& tokens,
                               const Range& input_vars = std::array<std::string, 0>{});

/// @brief gives the Function and Variable names that intervene in this UAST
deps::Deps direct_dependencies(const UAST& uast);

} // namespace parsing
} // namespace zc
