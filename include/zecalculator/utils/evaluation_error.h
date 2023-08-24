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

#include <zecalculator/utils/syntax_tree.h>
#include <zecalculator/utils/token.h>

namespace zc
{

struct EvaluationError
{
  // type of error
  enum Type : uint8_t
  {
    UNDEFINED = 0,
    UNDEFINED_VARIABLE,
    UNDEFINED_FUNCTION,
    CALLING_FUN_ARG_COUNT_MISMATCH,
    CALLED_FUN_ARG_COUNT_MISMATCH,
    NOT_IMPLEMENTED,
    EMPTY_EXPRESSION,
    INVALID_FUNCTION,
    CALLING_INVALID_FUNCTION, // expression that contains a function who cannot return values
    RECURSION_DEPTH_OVERFLOW, // maximum recursion depth has been reached
    WRONG_OBJECT_TYPE, // object has been used as a different type as it actually is, example "2+cos" (where cos is a function used here as variable)
  };

  static EvaluationError undefined_variable(tokens::Text tokenTxt)
  {
    return EvaluationError {UNDEFINED_VARIABLE, tokenTxt};
  }

  static EvaluationError undefined_function(tokens::Text tokenTxt)
  {
    return EvaluationError {UNDEFINED_FUNCTION, tokenTxt};
  }

  static EvaluationError mismatched_fun_args(tokens::Text tokenTxt)
  {
    return EvaluationError {CALLING_FUN_ARG_COUNT_MISMATCH, tokenTxt};
  }

  static EvaluationError mismatched_fun_args()
  {
    return EvaluationError {CALLED_FUN_ARG_COUNT_MISMATCH};
  }

  static EvaluationError not_implemented(tokens::Text tokenTxt)
  {
    return EvaluationError {NOT_IMPLEMENTED, tokenTxt};
  }

  static EvaluationError empty_expression()
  {
    return EvaluationError {EMPTY_EXPRESSION};
  }

  static EvaluationError invalid_function()
  {
    return EvaluationError {INVALID_FUNCTION};
  }

  static EvaluationError calling_invalid_function(tokens::Text tokenTxt)
  {
    return EvaluationError {CALLING_INVALID_FUNCTION, tokenTxt};
  }

  static EvaluationError recursion_depth_overflow()
  {
    return EvaluationError{RECURSION_DEPTH_OVERFLOW};
  }

  static EvaluationError wrong_object_type(tokens::Text tokenTxt)
  {
    return EvaluationError {WRONG_OBJECT_TYPE, tokenTxt};
  }

  // kind of error
  Type error_type = UNDEFINED;

  // on what token
  tokens::Text node = {};

  bool operator == (const EvaluationError& other) const = default;
};

}
