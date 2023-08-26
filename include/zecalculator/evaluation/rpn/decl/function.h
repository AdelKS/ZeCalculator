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

#include <zecalculator/evaluation/error.h>
#include <zecalculator/mathworld/mathworld.h>
#include <zecalculator/math_objects/decl/sequence.h>

namespace zc {
namespace eval {
namespace rpn {

struct Function
{
  const zc::rpn::MathWorld& world;
  const zc::parsing::tokens::Function& func_token;
  tl::expected<std::vector<double>, zc::eval::Error>& expected_eval_stack;
  const size_t current_recursion_depth;

  void operator () (zc::rpn::MathWorld::UnregisteredObject);

  void operator () (const zc::rpn::MathWorld::ConstMathObject<CppUnaryFunction>& function);

  void operator () (const zc::rpn::MathWorld::ConstMathObject<CppBinaryFunction>& function);

  void operator()(const zc::rpn::MathWorld::ConstMathObject<zc::rpn::Function>& function);

  void operator()(const zc::rpn::MathWorld::ConstMathObject<zc::rpn::Sequence>& sequence);

  void operator()(const auto&);
};

}
}
}
