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

#include <zecalculator/evaluation/ast/decl/function.h>

#include <zecalculator/math_objects/impl/sequence.h>

namespace zc {
namespace eval {
namespace ast {

inline Function::ReturnType Function::operator()(zc::ast::MathWorld::UnregisteredObject)
{
  return tl::unexpected(Error::undefined_function(node));
}

inline Function::ReturnType
  Function::operator()(const zc::ast::MathWorld::ConstMathObject<CppUnaryFunction>& function)
{
  if (evaluations.size() != 1)
    return tl::unexpected(Error::mismatched_fun_args(node));
  else
    return (*function)(evaluations.front());
}

inline Function::ReturnType
  Function::operator()(const zc::ast::MathWorld::ConstMathObject<CppBinaryFunction>& function)
{
  if (evaluations.size() != 2)
    return tl::unexpected(Error::mismatched_fun_args(node));
  else
    return (*function)(evaluations.front(), evaluations.back());
}

inline Function::ReturnType
  Function::operator()(const zc::ast::MathWorld::ConstMathObject<zc::ast::Function>& function)
{
  //              std::cout << "Evaluating zc function: " << node.name << std::endl;
  if (not bool(*function))
    return tl::unexpected(Error::calling_invalid_function(node));
  else if (evaluations.size() != function->argument_size().value())
    return tl::unexpected(Error::mismatched_fun_args(node));
  else
  {
    return function->evaluate(evaluations, world, current_recursion_depth + 1);
  }
}

inline Function::ReturnType
  Function::operator()(const zc::ast::MathWorld::ConstMathObject<zc::ast::Sequence>& sequence)
{
  //              std::cout << "Evaluating zc function: " << node.name << std::endl;
  if (not bool(*sequence))
    return tl::unexpected(Error::calling_invalid_function(node));
  else if (evaluations.size() != sequence->argument_size().value())
    return tl::unexpected(Error::mismatched_fun_args(node));
  else
  {
    // sequence handles only one argument
    return sequence->evaluate(evaluations.front(), world, current_recursion_depth + 1);
  }
}

inline Function::ReturnType Function::operator()(const auto&)
{
  return tl::unexpected(Error::wrong_object_type(node));
}

}
}
}
