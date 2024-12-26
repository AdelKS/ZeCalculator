#pragma once

#include <zecalculator/math_objects/decl/data.h>
#include <zecalculator/evaluation/impl/evaluation.h>
#include <zecalculator/parsing/impl/utils.h>

namespace zc {

template <parsing::Type type>
Data<type>::Data(std::string name,
                 std::string input_var_name,
                 std::vector<std::string> str_data,
                 const MathWorld<type>* math_world)
  : name(std::move(name)), input_var_name(std::move(input_var_name)), math_world(math_world)
{
  set_data(std::move(str_data));
}

template <parsing::Type type>
Data<type>& Data<type>::set_data(std::vector<std::string> string_data)
{
  values.resize(string_data.size(), tl::unexpected(zc::Error::empty_expression()));
  str_data.resize(string_data.size());
  expr_asts.resize(string_data.size(), tl::unexpected(zc::Error::empty_expression()));

  for (size_t i = 0 ; i != str_data.size() ; i++)
    set_expression(i, std::move(string_data[i]));

  return *this;
}

template <parsing::Type type>
Data<type>& Data<type>::set_expression(size_t index, std::string expr)
{
  assert(values.size() == str_data.size());

  if (index >= str_data.size())
    str_data.resize(index+1);

  if (index >= values.size())
    values.resize(index+1, tl::unexpected(zc::Error::empty_expression()));

  if (index >= expr_asts.size())
    expr_asts.resize(index+1, tl::unexpected(zc::Error::empty_expression()));

  str_data[index] = std::move(expr);

  expr_asts[index] = parsing::tokenize(str_data[index])
                       .and_then(parsing::make_ast{str_data[index]})
                       .transform(parsing::flatten_separators);

  rebind_at(index);

  return *this;
}

template <parsing::Type type>
void Data<type>::rebind_at(size_t index)
{
  assert(math_world); // should be non-nullptr

  auto input_var_arr = std::array{std::string_view(input_var_name)};
  if constexpr (type == parsing::Type::FAST)
    values[index] = expr_asts[index]
                      .transform(parsing::mark_input_vars{input_var_arr})
                      .and_then(parsing::make_fast<type>{str_data[index], *math_world});
  else
    values[index] = expr_asts[index]
                      .transform(parsing::mark_input_vars{input_var_arr})
                      .and_then(parsing::make_fast<type>{str_data[index], *math_world})
                      .transform(parsing::make_RPN);
}

template <parsing::Type type>
tl::expected<double, Error> Data<type>::evaluate(double index,
                                                 size_t current_recursion_depth,
                                                 eval::Cache* cache) const
{
  // round double to nearest integer
  double rounded_index = std::round(index);

  if (rounded_index < 0 or values.empty()) [[unlikely]]
    return std::nan("");

  if (cache)
    if (auto obj_cache_it = cache->find(name); obj_cache_it != cache->end())
    {
      auto& obj_cache = obj_cache_it->second.get_cache();
      if (auto value_it = obj_cache.find(rounded_index); value_it != obj_cache.end())
        return value_it->second;
    }

  size_t unsigned_index = rounded_index;
  const auto& parsing = unsigned_index < values.size()
                          ? values[unsigned_index]
                          : tl::unexpected(zc::Error::empty_expression());

  auto exp_res = parsing ? zc::evaluate(*parsing,
                                        std::array{rounded_index},
                                        current_recursion_depth,
                                        cache)
                         : tl::unexpected(parsing.error());

  if (exp_res and cache)
    (*cache)[name].insert(rounded_index, *exp_res);

  return exp_res;
}

template <parsing::Type type>
tl::expected<double, Error> Data<type>::evaluate(double index, eval::Cache* cache) const
{
  return Data<type>::evaluate(index, 0, cache);
}

template <parsing::Type type>
tl::expected<double, Error> Data<type>::operator ()(double index, eval::Cache* cache) const
{
  return Data<type>::evaluate(index, 0, cache);
}

template <parsing::Type type>
void Data<type>::rebind_dependent_expressions(const std::unordered_set<std::string>& names)
{
  auto input_var_arr = std::array{std::string_view(input_var_name)};
  for (size_t i = 0 ; i != expr_asts.size(); i++)
  {
    auto&& ast = expr_asts[i];
    if (ast)
    {
      auto direct_deps = zc::parsing::direct_dependencies(
        zc::parsing::mark_input_vars{input_var_arr}(*ast));

      if (std::ranges::any_of(names, [&](auto&& name){ return direct_deps.contains(name); }))
        rebind_at(i);
    }
  }
}

}
