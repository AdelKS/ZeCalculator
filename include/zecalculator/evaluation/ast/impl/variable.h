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

#include <zecalculator/evaluation/ast/decl/variable.h>

#include <zecalculator/math_objects/impl/expression.h>

namespace zc {
namespace eval {
namespace ast {

inline Variable::ReturnType Variable::operator()(zc::ast::MathWorld::UnregisteredObject)
{
  return tl::unexpected(Error::undefined_variable(node));
}

inline Variable::ReturnType
  Variable::operator()(const zc::ast::MathWorld::ConstMathObject<GlobalConstant>& global_constant)
{
  return global_constant->value;
}

inline Variable::ReturnType
  Variable::operator()(const zc::ast::MathWorld::ConstMathObject<zc::ast::GlobalVariable>& global_variable)
{
  return global_variable->evaluate(world, current_recursion_depth + 1);
}

inline Variable::ReturnType Variable::operator()(const auto&)
{
  return tl::unexpected(Error::wrong_object_type(node));
}

}
}
}
