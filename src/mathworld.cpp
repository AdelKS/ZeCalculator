#include <zecalculator/mathworld.h>

#include <cmath>
#include <ranges>
#include <algorithm>

namespace zc {

const MathWorld MathWorld::default_world = MathWorld(builtin_unary_functions,
                                                     builtin_binary_functions,
                                                     builtin_global_variables);

MathWorld global_world;

size_t MathWorld::add_function(std::string_view name)
{
  // if an object of this name exists, return an empty optional
  if (contains(name))
    throw name_already_taken(std::string(name));

  size_t function_index = functions.push(Function());
  inventory.insert({std::string(name), {FUNCTION, function_index}});
  return function_index;
}

}
