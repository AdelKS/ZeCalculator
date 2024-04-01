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

#include <cstdint>
#include <zecalculator/parsing/data_structures/token.h>

namespace zc {

struct Error
{
  // type of error
  enum Type : uint8_t
  {
    CALLING_FUN_ARG_COUNT_MISMATCH,
    OBJECT_INVALID_STATE, // expression that contains a function who cannot return values
    EMPTY_EXPRESSION,
    EMPTY,
    MISSING,
    NAME_ALREADY_TAKEN,
    NOT_IMPLEMENTED,
    OBJECT_NOT_IN_WORLD,
    RECURSION_DEPTH_OVERFLOW, // maximum recursion depth has been reached
    UNDEFINED_FUNCTION,
    UNDEFINED_VARIABLE,
    UNEXPECTED,
    UNKNOWN,
    WRONG_FORMAT,
    WRONG_OBJECT_TYPE, // object has been used as a different type as it actually is, example "2+cos" (where cos is a function used here as variable)
    NOT_MATH_OBJECT_DEFINITION, // the parsed expression is not of the form "[func_call] = [expression]" or " [variable_name] = [expression]"
    CPP_INCORRECT_ARGNUM, // programmatically evaluating math object with incorrect number of arguments
  };

  static Error empty()
  {
    return Error {EMPTY};
  }

  static Error unexpected(parsing::tokens::Text  token, std::string expression)
  {
    return Error {UNEXPECTED, token, std::move(expression)};
  }

  static Error wrong_format(parsing::tokens::Text  token, std::string expression)
  {
    return Error {WRONG_FORMAT, token, std::move(expression)};
  }

  static Error missing(parsing::tokens::Text  token, std::string expression)
  {
    return Error {MISSING, token, std::move(expression)};
  }

  static Error unkown()
  {
    return Error {UNKNOWN};
  }

  static Error undefined_variable(parsing::tokens::Text tokenTxt, std::string expression)
  {
    return Error {UNDEFINED_VARIABLE, tokenTxt, std::move(expression)};
  }

  static Error undefined_function(parsing::tokens::Text tokenTxt, std::string expression)
  {
    return Error {UNDEFINED_FUNCTION, tokenTxt, std::move(expression)};
  }

  static Error cpp_incorrect_argnum()
  {
    return Error {CPP_INCORRECT_ARGNUM};
  }

  static Error mismatched_fun_args(parsing::tokens::Text tokenTxt, std::string expression)
  {
    return Error {CALLING_FUN_ARG_COUNT_MISMATCH, tokenTxt, std::move(expression)};
  }

  static Error not_implemented(parsing::tokens::Text tokenTxt, std::string expression)
  {
    return Error {NOT_IMPLEMENTED, tokenTxt, std::move(expression)};
  }

  static Error empty_expression()
  {
    return Error {EMPTY_EXPRESSION};
  }

  static Error object_in_invalid_state(parsing::tokens::Text tokenTxt, std::string expression)
  {
    return Error {OBJECT_INVALID_STATE, tokenTxt, std::move(expression)};
  }

  static Error recursion_depth_overflow()
  {
    return Error{RECURSION_DEPTH_OVERFLOW};
  }

  static Error wrong_object_type(parsing::tokens::Text tokenTxt, std::string expression)
  {
    return Error {WRONG_OBJECT_TYPE, tokenTxt, std::move(expression)};
  }

  static Error name_already_taken(parsing::tokens::Text tokenTxt, std::string expression)
  {
    return Error {NAME_ALREADY_TAKEN, tokenTxt, std::move(expression)};
  }

  static Error object_not_in_world(parsing::tokens::Text tokenTxt, std::string expression)
  {
    return Error {OBJECT_NOT_IN_WORLD, tokenTxt, std::move(expression)};
  }

  static Error object_not_in_world()
  {
    return Error {OBJECT_NOT_IN_WORLD};
  }

  static Error not_math_object_definition()
  {
    return Error {NOT_MATH_OBJECT_DEFINITION};
  };

  // kind of error
  Type type = UNKNOWN;

  // on what token
  parsing::tokens::Text token = {};

  /// @brief full expression where the parsing error is
  std::string expression = {};

  bool operator == (const Error& other) const = default;
};

}
