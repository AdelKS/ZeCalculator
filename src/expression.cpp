#include <zecalculator/expression.h>

namespace zc {

tl::expected<double, EvaluationError> Expression::evaluate(const MathWorld& world) const
{
  return Function::evaluate({}, world);
}

tl::expected<double, EvaluationError> Expression::operator ()(const MathWorld& world) const
{
  return Function::evaluate({}, world);
}

}
