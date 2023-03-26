#include <zecalculator/mathworld.h>

#include <cmath>
#include <ranges>
#include <algorithm>

namespace zc {

const MathWorld MathWorld::default_world = MathWorld(builtin_unary_functions, builtin_binary_functions);

MathWorld global_world;

}
