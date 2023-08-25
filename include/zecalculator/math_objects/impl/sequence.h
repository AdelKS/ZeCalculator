#pragma once

#include <zecalculator/math_objects/decl/sequence.h>

namespace zc {

template <parsing::Type type>
Sequence<type>::Sequence(std::string var_name,
                         const std::string& expr,
                         std::vector<double> first_vals)
  : Function<type>(std::vector{var_name}, expr), values(first_vals)
{}

template <parsing::Type type>
void Sequence<type>::set_expression(const std::string& expr)
{
  Function<type>::set_expression(expr);
  values.clear();
}

template <parsing::Type type>
void Sequence<type>::set_input_var(std::string var_name)
{
  this->set_input_vars(std::vector {var_name});
}

template <parsing::Type type>
void Sequence<type>::set_first_values(std::vector<double> first_vals)
{
  values = std::move(first_vals);
}

template <parsing::Type type>
void Sequence<type>::set_first_val_index(int index)
{
  first_val_index = index;
  values.clear();
}

template <parsing::Type type>
constexpr int Sequence<type>::get_first_val_index() const { return first_val_index; };

template <parsing::Type type>
tl::expected<double, eval::Error> Sequence<type>::evaluate(double index,
                                                           const MathWorld& world,
                                                           size_t current_recursion_depth) const
{
  // round double to nearest integer
  int integer_index = std::lround(index);

  if (integer_index < first_val_index) [[unlikely]]
    return std::nan("");
  else if (size_t(integer_index - first_val_index) < values.size())
    return values[size_t(integer_index - first_val_index)];
  else return Function<type>::evaluate({double(integer_index)}, world, current_recursion_depth);
}

template <parsing::Type type>
tl::expected<double, eval::Error> Sequence<type>::evaluate(double index, const MathWorld& world) const
{
  return evaluate(index, world, 0);
}

template <parsing::Type type>
tl::expected<double, eval::Error> Sequence<type>::operator ()(double index, const MathWorld& world) const
{
  return evaluate(index, world);
}

}
