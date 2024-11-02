#pragma once

#include <zecalculator/math_objects/decl/sequence.h>
#include <zecalculator/evaluation/impl/evaluation.h>

namespace zc {

template <parsing::Type type>
Sequence<type>::Sequence(MathObject obj, std::string equation)
  : MathObject(std::move(obj)), equation(std::move(equation))
{}

template <parsing::Type type>
Sequence<type>::Sequence(MathObject obj, std::string equation, std::vector<parsing::Parsing<type>> values)
  : MathObject(std::move(obj)), equation(std::move(equation)), values(std::move(values))
{}

template <parsing::Type type>
tl::expected<double, Error> Sequence<type>::evaluate(double index,
                                                     size_t current_recursion_depth,
                                                     eval::Cache* cache) const
{
  // round double to nearest integer
  long integer_index = std::lround(index);

  assert(values.size() != 0);

  if (integer_index < 0) [[unlikely]]
    return std::nan("");

  const auto& parsing = size_t(integer_index) < values.size() ? values[integer_index] : values.back();

  return zc::evaluate(parsing, std::array{double(integer_index)}, current_recursion_depth, cache);
}

template <parsing::Type type>
tl::expected<double, Error> Sequence<type>::evaluate(double index, eval::Cache* cache) const
{
  return Sequence<type>::evaluate(index, 0, cache);
}

template <parsing::Type type>
tl::expected<double, Error> Sequence<type>::operator ()(double index, eval::Cache* cache) const
{
  return Sequence<type>::evaluate(index, 0, cache);
}

}
