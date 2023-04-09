#include <zecalculator/mathworld.h>
#include <zecalculator/utils/evaluation.h>

#include <cmath>
#include <ranges>
#include <algorithm>

namespace zc {

const MathWorld MathWorld::default_world = MathWorld(builtin_unary_functions,
                                                     builtin_binary_functions,
                                                     builtin_global_variables);

MathWorld global_world;

MathWorld::WorldFunction MathWorld::add_function(std::string_view name)
{
  // if an object of this name exists, return an empty optional
  if (contains(name))
    throw name_already_taken(std::string(name));

  size_t function_index = functions.push(Function());
  inventory.insert({std::string(name), {FUNCTION, function_index}});
  return WorldFunction(*this, function_index);
}

tl::expected<double, EvaluationError>
  MathWorld::WorldFunction::evaluate(const std::vector<double>& args) const
{
  return world.get_function(id).evaluate(args, world);
}

tl::expected<double, EvaluationError>
  MathWorld::WorldFunction::operator()(const std::vector<double>& args) const
{
  return evaluate(args);
}

}
