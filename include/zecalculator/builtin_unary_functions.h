#pragma once

#include <string_view>
#include <array>
#include <cmath>

namespace zc {

using BuiltinUnaryFunction = double (*) (double);

// we save the names along with the function pointers for convenience
// we could save only the function pointers, and the names only in the inventory
constexpr std::array<std::pair<std::string_view, BuiltinUnaryFunction>, 31> builtin_unary_functions = {{
  {"cos",   static_cast<BuiltinUnaryFunction>(std::cos)},
  {"sin",   static_cast<BuiltinUnaryFunction>(std::sin)},
  {"tan",   static_cast<BuiltinUnaryFunction>(std::tan)},

  {"acos",  static_cast<BuiltinUnaryFunction>(std::acos)},
  {"asin",  static_cast<BuiltinUnaryFunction>(std::asin)},
  {"atan",  static_cast<BuiltinUnaryFunction>(std::atan)},

  {"cosh",  static_cast<BuiltinUnaryFunction>(std::cosh)},
  {"sinh",  static_cast<BuiltinUnaryFunction>(std::sinh)},
  {"tanh",  static_cast<BuiltinUnaryFunction>(std::tanh)},

  {"ch",    static_cast<BuiltinUnaryFunction>(std::cosh)},
  {"sh",    static_cast<BuiltinUnaryFunction>(std::sinh)},
  {"th",    static_cast<BuiltinUnaryFunction>(std::tanh)},

  {"acosh", static_cast<BuiltinUnaryFunction>(std::acosh)},
  {"asinh", static_cast<BuiltinUnaryFunction>(std::asinh)},
  {"atanh", static_cast<BuiltinUnaryFunction>(std::atanh)},

  {"ach",   static_cast<BuiltinUnaryFunction>(std::acosh)},
  {"ash",   static_cast<BuiltinUnaryFunction>(std::asinh)},
  {"ath",   static_cast<BuiltinUnaryFunction>(std::atanh)},

  {"sqrt",  static_cast<BuiltinUnaryFunction>(std::sqrt)},
  {"log",   static_cast<BuiltinUnaryFunction>(std::log10)},
  {"lg",    static_cast<BuiltinUnaryFunction>(std::log2)},
  {"ln",    static_cast<BuiltinUnaryFunction>(std::log)},
  {"abs",   static_cast<BuiltinUnaryFunction>(std::abs)},
  {"exp",   static_cast<BuiltinUnaryFunction>(std::exp)},
  {"floor", static_cast<BuiltinUnaryFunction>(std::floor)},
  {"ceil",  static_cast<BuiltinUnaryFunction>(std::ceil)},
  {"erf",   static_cast<BuiltinUnaryFunction>(std::erf)},
  {"erfc",  static_cast<BuiltinUnaryFunction>(std::erfc)},
  {"gamma", static_cast<BuiltinUnaryFunction>(std::tgamma)},
  {"Γ",     static_cast<BuiltinUnaryFunction>(std::tgamma)},
}};

}
