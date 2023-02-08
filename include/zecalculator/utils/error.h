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

struct Error
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
  static Error unexpected(Token::Type token_type, std::string_view where)
  {
    return Error {.error_type = UNEXPECTED, .token_type = token_type, .where = where};
  }

  /// @brief creates an "wrong_format" typed error
  static Error wrong_format(Token::Type token_type, std::string_view where)
  {
    return Error {.error_type = WRONG_FORMAT, .token_type = token_type, .where = where};
  }

  /// @brief creates an "missing" typed error
  static Error missing(Token::Type token_type, std::string_view where)
  {
    return Error {.error_type = MISSING, .token_type = token_type, .where = where};
  }

  // kind of error
  Type error_type = UNDEFINED;

  // on what token
  Token::Type token_type = Token::UNKNOWN;

  // where in the expression
  std::string_view where;
};

}
