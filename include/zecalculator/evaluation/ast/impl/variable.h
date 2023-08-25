#pragma once

#include <zecalculator/evaluation/ast/decl/variable.h>

#include <zecalculator/math_objects/impl/expression.h>

namespace zc {
namespace eval {

inline Variable::ReturnType Variable::operator()(ast::MathWorld::UnregisteredObject)
{
  return tl::unexpected(Error::undefined_variable(node));
}

inline Variable::ReturnType
  Variable::operator()(const ast::MathWorld::ConstMathObject<GlobalConstant>& global_constant)
{
  return global_constant->value;
}

inline Variable::ReturnType
  Variable::operator()(const ast::MathWorld::ConstMathObject<ast::GlobalVariable>& global_variable)
{
  return global_variable->evaluate(world, current_recursion_depth + 1);
}

inline Variable::ReturnType Variable::operator()(const auto&)
{
  return tl::unexpected(Error::wrong_object_type(node));
}

}
}
