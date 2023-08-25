#pragma once

#include <zecalculator/mathworld.h>
#include <zecalculator/evaluation/error.h>

namespace zc {
namespace eval {

struct Function
{
  const MathWorld& world;
  const ast::node::Function& node;
  const std::vector<double>& evaluations;
  const size_t current_recursion_depth;

  using ReturnType = tl::expected<double, Error>;

  ReturnType operator () (MathWorld::UnregisteredObject)
  {
    return tl::unexpected(Error::undefined_function(node));
  }

  ReturnType operator () (const MathWorld::ConstMathObject<CppUnaryFunction>& function)
  {
    if (evaluations.size() != 1)
      return tl::unexpected(Error::mismatched_fun_args(node));
    else
      return (*function)(evaluations.front());
  }

  ReturnType operator () (const MathWorld::ConstMathObject<CppBinaryFunction>& function)
  {
    if (evaluations.size() != 2)
      return tl::unexpected(Error::mismatched_fun_args(node));
    else
      return (*function)(evaluations.front(), evaluations.back());
  }

  ReturnType operator()(const MathWorld::ConstMathObject<zc::Function>& function)
  {
    //              std::cout << "Evaluating zc function: " << node.name << std::endl;
    if (not bool(*function))
      return tl::unexpected(Error::calling_invalid_function(node));
    else if (evaluations.size() != function->argument_size())
      return tl::unexpected(Error::mismatched_fun_args(node));
    else
    {
      return function->evaluate(evaluations, world, current_recursion_depth + 1);
    }
  }

  ReturnType operator()(const MathWorld::ConstMathObject<zc::Sequence>& sequence)
  {
    //              std::cout << "Evaluating zc function: " << node.name << std::endl;
    if (not bool(*sequence))
      return tl::unexpected(Error::calling_invalid_function(node));
    else if (evaluations.size() != sequence->argument_size())
      return tl::unexpected(Error::mismatched_fun_args(node));
    else
    {
      // sequence handles only one argument
      return sequence->evaluate(evaluations.front(), world, current_recursion_depth + 1);
    }
  }

  ReturnType operator()(const auto&)
  {
    return tl::unexpected(Error::wrong_object_type(node));
  }
};

}
}
