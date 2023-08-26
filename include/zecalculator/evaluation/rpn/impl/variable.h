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

#include <zecalculator/evaluation/rpn/decl/variable.h>

#include <zecalculator/math_objects/impl/expression.h>

namespace zc {
namespace eval {
namespace rpn {

inline void Variable::operator()(zc::rpn::MathWorld::UnregisteredObject)
{
  expected_eval_stack = tl::unexpected(Error::undefined_variable(var_token));
}

inline void
  Variable::operator()(const zc::rpn::MathWorld::ConstMathObject<GlobalConstant>& global_constant)
{
  expected_eval_stack->push_back(global_constant->value);
}

inline void Variable::operator()(
  const zc::rpn::MathWorld::ConstMathObject<zc::rpn::GlobalVariable>& global_variable)
{
  auto expected_res = global_variable->evaluate(world, current_recursion_depth + 1);
  if (expected_res)
    expected_eval_stack->push_back(*expected_res);
  else expected_eval_stack = tl::unexpected(expected_res.error());
}

inline void Variable::operator()(const auto&)
{
  expected_eval_stack = tl::unexpected(Error::wrong_object_type(var_token));
}

}
}
}
