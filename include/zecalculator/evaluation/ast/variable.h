#pragma once

#include <zecalculator/mathworld.h>
#include <zecalculator/evaluation/error.h>

namespace zc {
namespace eval {

struct Variable
{
  const MathWorld& world;
  const ast::node::Variable& node;
  const size_t current_recursion_depth;

  using ReturnType = tl::expected<double, Error>;

  ReturnType operator () (MathWorld::UnregisteredObject)
  {
    return tl::unexpected(Error::undefined_variable(node));
  }

  ReturnType operator () (const MathWorld::ConstMathObject<GlobalConstant>& global_constant)
  {
    return global_constant->value;
  }

  ReturnType operator () (const MathWorld::ConstMathObject<GlobalVariable>& global_variable)
  {
    return global_variable->evaluate(world, current_recursion_depth + 1);
  }

  ReturnType operator () (const auto&)
  {
    return tl::unexpected(Error::wrong_object_type(node));
  }
};

}
}
