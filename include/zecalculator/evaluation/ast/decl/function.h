#pragma once

#include <zecalculator/evaluation/error.h>
#include <zecalculator/mathworld/mathworld.h>
#include <zecalculator/math_objects/decl/sequence.h>

namespace zc {
namespace eval {

struct Function
{
  const ast::MathWorld& world;
  const ast::node::Function& node;
  const std::vector<double>& evaluations;
  const size_t current_recursion_depth;

  using ReturnType = tl::expected<double, Error>;

  ReturnType operator () (ast::MathWorld::UnregisteredObject);

  ReturnType operator () (const ast::MathWorld::ConstMathObject<CppUnaryFunction>& function);

  ReturnType operator () (const ast::MathWorld::ConstMathObject<CppBinaryFunction>& function);

  ReturnType operator()(const ast::MathWorld::ConstMathObject<zc::ast::Function>& function);

  ReturnType operator()(const ast::MathWorld::ConstMathObject<zc::ast::Sequence>& sequence);

  ReturnType operator()(const auto&);
};

}
}
