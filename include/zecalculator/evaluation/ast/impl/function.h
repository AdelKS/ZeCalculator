#pragma once

#include <zecalculator/evaluation/ast/decl/function.h>

#include <zecalculator/math_objects/impl/sequence.h>

namespace zc {
namespace eval {

inline Function::ReturnType Function::operator () (MathWorld::UnregisteredObject)
{
  return tl::unexpected(Error::undefined_function(node));
}

inline Function::ReturnType Function::operator () (const MathWorld::ConstMathObject<CppUnaryFunction>& function)
{
  if (evaluations.size() != 1)
    return tl::unexpected(Error::mismatched_fun_args(node));
  else
    return (*function)(evaluations.front());
}

inline Function::ReturnType Function::operator () (const MathWorld::ConstMathObject<CppBinaryFunction>& function)
{
  if (evaluations.size() != 2)
    return tl::unexpected(Error::mismatched_fun_args(node));
  else
    return (*function)(evaluations.front(), evaluations.back());
}

inline Function::ReturnType Function::operator()(const MathWorld::ConstMathObject<zc::Function>& function)
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

inline Function::ReturnType Function::operator()(const MathWorld::ConstMathObject<zc::Sequence>& sequence)
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

inline Function::ReturnType Function::operator()(const auto&)
{
  return tl::unexpected(Error::wrong_object_type(node));
}

}
}
