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

#include <zecalculator/error.h>
#include <zecalculator/mathworld/mathworld.h>
#include <zecalculator/math_objects/global_variable.h>

namespace zc {
namespace eval {
namespace rpn {

struct Variable
{
  const zc::rpn::MathWorld& world;
  const parsing::tokens::Variable& var_token;
  tl::expected<std::vector<double>, zc::Error>& expected_eval_stack;
  const size_t current_recursion_depth;

  void operator () (zc::rpn::MathWorld::UnregisteredObject);

  void operator () (const zc::rpn::MathWorld::ConstMathObject<GlobalConstant>& global_constant);

  void operator () (const zc::rpn::MathWorld::ConstMathObject<zc::rpn::GlobalVariable>& global_variable);

  void operator () (const auto&);
};

}
}
}
