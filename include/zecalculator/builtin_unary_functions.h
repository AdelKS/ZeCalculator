#pragma once

#include <string_view>
#include <array>
#include <cmath>

namespace zc {

using CppUnaryFunction = double (*) (double);

// we save the names along with the function pointers for convenience
// we could save only the function pointers, and the names only in the inventory
constexpr std::array<std::pair<std::string_view, CppUnaryFunction>, 31> builtin_unary_functions = {{
  {"cos",   static_cast<CppUnaryFunction>(std::cos)},
  {"sin",   static_cast<CppUnaryFunction>(std::sin)},
  {"tan",   static_cast<CppUnaryFunction>(std::tan)},

  {"acos",  static_cast<CppUnaryFunction>(std::acos)},
  {"asin",  static_cast<CppUnaryFunction>(std::asin)},
  {"atan",  static_cast<CppUnaryFunction>(std::atan)},

  {"cosh",  static_cast<CppUnaryFunction>(std::cosh)},
  {"sinh",  static_cast<CppUnaryFunction>(std::sinh)},
  {"tanh",  static_cast<CppUnaryFunction>(std::tanh)},

  {"ch",    static_cast<CppUnaryFunction>(std::cosh)},
  {"sh",    static_cast<CppUnaryFunction>(std::sinh)},
  {"th",    static_cast<CppUnaryFunction>(std::tanh)},

  {"acosh", static_cast<CppUnaryFunction>(std::acosh)},
  {"asinh", static_cast<CppUnaryFunction>(std::asinh)},
  {"atanh", static_cast<CppUnaryFunction>(std::atanh)},

  {"ach",   static_cast<CppUnaryFunction>(std::acosh)},
  {"ash",   static_cast<CppUnaryFunction>(std::asinh)},
  {"ath",   static_cast<CppUnaryFunction>(std::atanh)},

  {"sqrt",  static_cast<CppUnaryFunction>(std::sqrt)},
  {"log",   static_cast<CppUnaryFunction>(std::log10)},
  {"lg",    static_cast<CppUnaryFunction>(std::log2)},
  {"ln",    static_cast<CppUnaryFunction>(std::log)},
  {"abs",   static_cast<CppUnaryFunction>(std::abs)},
  {"exp",   static_cast<CppUnaryFunction>(std::exp)},
  {"floor", static_cast<CppUnaryFunction>(std::floor)},
  {"ceil",  static_cast<CppUnaryFunction>(std::ceil)},
  {"erf",   static_cast<CppUnaryFunction>(std::erf)},
  {"erfc",  static_cast<CppUnaryFunction>(std::erfc)},
  {"gamma", static_cast<CppUnaryFunction>(std::tgamma)},
  {"Γ",     static_cast<CppUnaryFunction>(std::tgamma)},
}};

}
