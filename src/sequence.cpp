#include <zecalculator/sequence.h>

namespace zc {

tl::expected<double, EvaluationError> Sequence::evaluate(double index, const MathWorld& world) const
{
  // round double to nearest integer
  int integer_index = std::lround(index);

  if (integer_index < first_val_index) [[unlikely]]
    return std::nan("");
  else if (uint(integer_index - first_val_index) < values.size())
    return values[uint(integer_index - first_val_index)];
  else return Function::evaluate({double(integer_index)}, world);
}

tl::expected<double, EvaluationError> Sequence::operator ()(double index, const MathWorld& world) const
{
  return evaluate(index, world);
}

}
