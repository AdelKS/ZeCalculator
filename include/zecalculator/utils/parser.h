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

#include <zecalculator/utils/token.h>
#include <zecalculator/utils/parsing_error.h>

/* TODO: update approach as the following:
    - Parse: aka cut each atom in a formula
    - Evaluate type of atom: separator, number
    - Treat number in a special as to make 1.2E+33 as one atom
    - Check for validity
    - Enable setting custom names for functions and variables
    - Performance improvement: flatten trees
*/

namespace zc {

/// @brief interprets "view" as a floating number
/// @returns if successful, the interpreted double and the number of characters interpreted, otherwise empty
std::optional<std::pair<double, size_t>> to_double(std::string_view view);

/// @brief parses the expression into a list of tokens
/// @note the string that is void must remain valid for for the returned instance
///       to remain valid (for both a successful or unsuccessful  parsing)
///       as they contain sub-string views of the input view
tl::expected<std::vector<Token>, ParsingError> parse(std::string_view expression);

/// @brief tells if the string_view contains a valid math object name
bool is_valid_name(std::string_view name);

}
