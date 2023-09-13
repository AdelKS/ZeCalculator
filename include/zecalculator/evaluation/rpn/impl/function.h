#pragma once

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

#include "zecalculator/external/expected.h"
#include <concepts>
#include <type_traits>
#include <zecalculator/evaluation/rpn/decl/function.h>

#include <zecalculator/math_objects/impl/sequence.h>

namespace zc {
namespace eval {
namespace rpn {

inline void Function::operator () (zc::rpn::MathWorld::UnregisteredObject)
{
  expected_eval_stack = tl::unexpected(Error::undefined_function(func_token));
}

inline void Function::operator () (const zc::rpn::MathWorld::ConstMathObject<CppUnaryFunction>& function)
{
  if (expected_eval_stack->empty())
    expected_eval_stack = tl::unexpected(Error::mismatched_fun_args(func_token));
  else
  {
    double& back_val = expected_eval_stack->back();

    // overwrite the last value with the function evaluation on it
    back_val = (*function)(back_val);
  }
}

inline void Function::operator () (const zc::rpn::MathWorld::ConstMathObject<CppBinaryFunction>& function)
{
  if (expected_eval_stack->size() < 2) [[unlikely]]
    expected_eval_stack = tl::unexpected(Error::mismatched_fun_args(func_token));
  else
  {
    // points on the before last value on the stack
    const auto it = expected_eval_stack->end() - 2;

    // since the function pops two elements, then pushes back one
    // we can overwrite directly the value that will get replaced
    *it = (*function)(*it, *(it+1));

    // remove the last value, i.e. at it+1, as it got consumed
    expected_eval_stack->pop_back();
  }

}

inline void Function::operator()(const zc::rpn::MathWorld::ConstMathObject<zc::rpn::Function>& function)
{
  //              std::cout << "Evaluating zc function: " << node.name << std::endl;

  const size_t args_num = function->argument_size().value();
  if (not bool(*function)) [[unlikely]]
    expected_eval_stack = tl::unexpected(Error::calling_invalid_function(func_token));
  else if (expected_eval_stack->size() < args_num) [[unlikely]]
    expected_eval_stack = tl::unexpected(Error::mismatched_fun_args(func_token));
  else
  {
    const auto evaluations = std::span<const double>(expected_eval_stack->end() - args_num,
                                                     expected_eval_stack->end());

    auto expected_res = function->evaluate(evaluations, world, current_recursion_depth + 1);

    // when done, remove args_num from the back of the evaluation stack
    // note: the resize needs to be done before pushing in the new result
    expected_eval_stack->resize(expected_eval_stack->size() - args_num);

    if (expected_res)
      // push the result to the evaluation stack if correct
      expected_eval_stack->push_back(*expected_res);

    // save error in the expected otherwise
    else expected_eval_stack = tl::unexpected(expected_res.error());
  }
}

inline void Function::operator()(const zc::rpn::MathWorld::ConstMathObject<zc::rpn::Sequence>& sequence)
{
  //              std::cout << "Evaluating zc function: " << node.name << std::endl;
  if (not bool(*sequence))
    expected_eval_stack = tl::unexpected(Error::calling_invalid_function(func_token));
  else if (expected_eval_stack->size() < sequence->argument_size().value())
    expected_eval_stack = tl::unexpected(Error::mismatched_fun_args(func_token));
  else
  {
    // sequence handles only one argument
    double& back_val = expected_eval_stack->back();

    auto expected_res = sequence->evaluate(back_val, world, current_recursion_depth + 1);

    if (expected_res)
      // overwrite the top of the stack on computation success
      back_val = *expected_res;

    // save error in the expected otherwise
    else expected_eval_stack = tl::unexpected(expected_res.error());
  }
}

inline void Function::operator()(const auto&)
{
  expected_eval_stack = tl::unexpected(Error::wrong_object_type(func_token));
}

}
}
}
