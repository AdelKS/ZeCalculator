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
#include <zecalculator/math_objects/decl/sequence.h>

namespace zc {
namespace eval {
namespace ast {

struct Function
{
  const zc::ast::MathWorld& world;
  const zc::ast::node::Function& node;
  const std::vector<double>& evaluations;
  const size_t current_recursion_depth;

  using ReturnType = tl::expected<double, Error>;

  ReturnType operator () (zc::ast::MathWorld::UnregisteredObject);

  ReturnType operator () (const zc::ast::MathWorld::ConstMathObject<CppUnaryFunction>& function);

  ReturnType operator () (const zc::ast::MathWorld::ConstMathObject<CppBinaryFunction>& function);

  ReturnType operator()(const zc::ast::MathWorld::ConstMathObject<zc::ast::Function>& function);

  ReturnType operator()(const zc::ast::MathWorld::ConstMathObject<zc::ast::Sequence>& sequence);

  ReturnType operator()(const auto&);
};

}
}
}
