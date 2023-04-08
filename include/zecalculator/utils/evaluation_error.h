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
    MISMATCHING_FUNCTION_ARGS_NUM,
    NOT_IMPLEMENTED,
    EMPTY_EXPRESSION,
    INVALID_FUNCTION,
  };

  static EvaluationError undefined_variable(SyntaxTree tree)
  {
    return EvaluationError {UNDEFINED_VARIABLE, tree};
  }

  static EvaluationError undefined_function(SyntaxTree tree)
  {
    return EvaluationError {UNDEFINED_FUNCTION, tree};
  }

  static EvaluationError mismatched_fun_args(SyntaxTree tree)
  {
    return EvaluationError {MISMATCHING_FUNCTION_ARGS_NUM, tree};
  }

  static EvaluationError not_implemented(SyntaxTree tree)
  {
    return EvaluationError {NOT_IMPLEMENTED, tree};
  }

  static EvaluationError empty_expression()
  {
    return EvaluationError {EMPTY_EXPRESSION};
  }

  static EvaluationError invalid_function()
  {
    return EvaluationError {INVALID_FUNCTION};
  }

  // kind of error
  Type error_type = UNDEFINED;

  // on what token
  SyntaxTree node = {};

  bool operator == (const EvaluationError& other) const = default;
};

}
