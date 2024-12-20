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
  double rounded_index = std::round(index);

  assert(values.size() != 0);

  if (rounded_index < 0) [[unlikely]]
    return std::nan("");

  if (cache)
    if (auto obj_cache_it = cache->find(name); obj_cache_it != cache->end())
    {
      auto& obj_cache = obj_cache_it->second.get_cache();
      if (auto value_it = obj_cache.find(rounded_index); value_it != obj_cache.end())
        return value_it->second;
    }

  size_t unsigned_index = rounded_index;
  const auto& parsing = unsigned_index < values.size() ? values[unsigned_index] : values.back();

  auto exp_res = zc::evaluate(parsing, std::array{rounded_index}, current_recursion_depth, cache);

  if (exp_res and cache)
    (*cache)[name].insert(rounded_index, *exp_res);

  return exp_res;
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
