#pragma once

#include <string_view>
#include <array>
#include <numbers>

namespace zc {

struct GlobalConstant
{
  constexpr GlobalConstant() = default;
  constexpr GlobalConstant(double val): value(val) {}
  double value = 0;
};

constexpr std::array<std::pair<std::string_view, GlobalConstant>, 5> builtin_global_variables =
{{
  {"math::pi", std::numbers::pi},
  {"math::π",  std::numbers::pi},
  {"physics::kB", 1.380649e-23},   // Blotzmann constant, SI units
  {"physics::h",  6.62607015e-34}, // Plank constant, SI units
  {"physics::c",  299792458},      // Speed of light in vacuum, SI units
}};

}
