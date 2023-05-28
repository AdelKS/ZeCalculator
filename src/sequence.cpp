#include <zecalculator/sequence.h>

namespace zc {

tl::expected<double, EvaluationError> Sequence::evaluate(double index,
                                                         const MathWorld& world,
                                                         size_t current_recursion_depth) const
{
  // round double to nearest integer
  int integer_index = std::lround(index);

  if (integer_index < first_val_index) [[unlikely]]
    return std::nan("");
  else if (uint(integer_index - first_val_index) < values.size())
    return values[uint(integer_index - first_val_index)];
  else return Function::evaluate({double(integer_index)}, world, current_recursion_depth);
}

tl::expected<double, EvaluationError> Sequence::evaluate(double index, const MathWorld& world) const
{
  return evaluate(index, world, 0);
}

tl::expected<double, EvaluationError> Sequence::operator ()(double index, const MathWorld& world) const
{
  return evaluate(index, world);
}

}
