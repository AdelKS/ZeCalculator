#pragma once

#include <zecalculator/evaluation/error.h>
#include <zecalculator/mathworld/mathworld.h>

namespace zc {
namespace eval {

struct Function
{
  const MathWorld& world;
  const ast::node::Function& node;
  const std::vector<double>& evaluations;
  const size_t current_recursion_depth;

  using ReturnType = tl::expected<double, Error>;

  ReturnType operator () (MathWorld::UnregisteredObject);

  ReturnType operator () (const MathWorld::ConstMathObject<CppUnaryFunction>& function);

  ReturnType operator () (const MathWorld::ConstMathObject<CppBinaryFunction>& function);

  ReturnType operator()(const MathWorld::ConstMathObject<zc::Function>& function);

  ReturnType operator()(const MathWorld::ConstMathObject<zc::Sequence>& sequence);

  ReturnType operator()(const auto&);
};

}
}
