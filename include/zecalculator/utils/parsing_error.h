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

#include <string_view>
#include <string>
#include <array>
#include <cassert>

#include <zecalculator/utils/token.h>

namespace zc
{

struct ParsingError
{
  // type of error
  enum Type : uint8_t
  {
    UNDEFINED = 0,
    WRONG_FORMAT,
    UNEXPECTED,
    MISSING,
  };

  /// @brief creates an "unexpected" typed error
  static ParsingError unexpected(Token token)
  {
    return ParsingError {.error_type = UNEXPECTED, .token = token};
  }

  /// @brief creates an "wrong_format" typed error
  static ParsingError wrong_format(Token token)
  {
    return ParsingError {.error_type = WRONG_FORMAT, .token = token};
  }

  /// @brief creates an "missing" typed error
  static ParsingError missing(Token token)
  {
    return ParsingError {.error_type = MISSING, .token = token};
  }

  // kind of error
  Type error_type = UNDEFINED;

  // on what token
  Token token;

  bool operator == (const ParsingError& other) const = default;
};

}
