#include <zecalculator/mathworld.h>

#include <cmath>
#include <ranges>
#include <algorithm>

namespace zc {

const MathWorld MathWorld::default_world = MathWorld(builtin_unary_functions,
                                                     builtin_binary_functions,
                                                     builtin_global_variables);

MathWorld global_world;

}
