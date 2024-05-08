#pragma once

#include <zecalculator/math_objects/decl/sequence.h>

namespace zc {

template <parsing::Type type>
Sequence<type>::Sequence(Parent parent) : Parent(std::move(parent))
{}

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
tl::expected<double, Error> Sequence<type>::evaluate(double index,
                                                     size_t current_recursion_depth) const
{
  // round double to nearest integer
  int integer_index = std::lround(index);

  if (integer_index < first_val_index) [[unlikely]]
    return std::nan("");
  else if (size_t(integer_index - first_val_index) < values.size())
    return values[size_t(integer_index - first_val_index)];
  else return Parent::evaluate(std::array{double(integer_index)}, current_recursion_depth);
}

template <parsing::Type type>
template <class DBL>
  requires std::is_constructible_v<DBL, double>
tl::expected<double, Error> Sequence<type>::evaluate(DBL index) const
{
  return Sequence<type>::evaluate(index, 0);
}

template <parsing::Type type>
template <class DBL>
  requires std::is_constructible_v<DBL, double>
tl::expected<double, Error> Sequence<type>::operator ()(DBL index) const
{
  return Sequence<type>::evaluate(index, 0);
}

}
