#pragma once

#include <zecalculator/math_objects/decl/sequence.h>

namespace zc {

inline Sequence::Sequence(std::string var_name,
                  const std::string& expr,
                  std::vector<double> first_vals)
  : Function(std::vector{var_name}, expr), values(first_vals)
{}

/// \brief set the expression
inline void Sequence::set_expression(const std::string& expr)
{
  Function::set_expression(expr);
  values.clear();
}

inline void Sequence::set_input_var(std::string var_name)
{
  set_input_vars(std::vector {var_name});
}

inline void Sequence::set_first_values(std::vector<double> first_vals)
{
  values = std::move(first_vals);
}

inline void Sequence::set_first_val_index(int index)
{
  first_val_index = index;
  values.clear();
}

constexpr int Sequence::get_first_val_index() const { return first_val_index; };

inline tl::expected<double, eval::Error> Sequence::evaluate(double index,
                                                         const MathWorld& world,
                                                         size_t current_recursion_depth) const
{
  // round double to nearest integer
  int integer_index = std::lround(index);

  if (integer_index < first_val_index) [[unlikely]]
    return std::nan("");
  else if (size_t(integer_index - first_val_index) < values.size())
    return values[size_t(integer_index - first_val_index)];
  else return Function::evaluate({double(integer_index)}, world, current_recursion_depth);
}

inline tl::expected<double, eval::Error> Sequence::evaluate(double index, const MathWorld& world) const
{
  return evaluate(index, world, 0);
}

inline tl::expected<double, eval::Error> Sequence::operator ()(double index, const MathWorld& world) const
{
  return evaluate(index, world);
}

}
