#pragma once

#include <zecalculator/mathworld.h>
#include <zecalculator/utils/evaluation_error.h>

namespace zc {

struct VariableEvaluator
{
  const MathWorld& world;
  const VariableNode& node;
  const size_t current_recursion_depth;

  using ReturnType = tl::expected<double, EvaluationError>;

  ReturnType operator () (MathWorld::UnregisteredObject)
  {
    return tl::unexpected(EvaluationError::undefined_variable(node));
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
    return tl::unexpected(EvaluationError::wrong_object_type(node));
  }
};

}
