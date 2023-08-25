#pragma once

#include <zecalculator/evaluation/error.h>
#include <zecalculator/mathworld/mathworld.h>
#include <zecalculator/math_objects/global_variable.h>

namespace zc {
namespace eval {

struct Variable
{
  const MathWorld& world;
  const ast::node::Variable& node;
  const size_t current_recursion_depth;

  using ReturnType = tl::expected<double, Error>;

  ReturnType operator () (MathWorld::UnregisteredObject);

  ReturnType operator () (const MathWorld::ConstMathObject<GlobalConstant>& global_constant);

  ReturnType operator () (const MathWorld::ConstMathObject<ast::GlobalVariable>& global_variable);

  ReturnType operator () (const auto&);
};

}
}
