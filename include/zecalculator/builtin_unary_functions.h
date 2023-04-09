#pragma once

#include <string_view>
#include <array>
#include <cmath>

namespace zc {

using CppUnaryFunctionPtr = double (*) (double);

class CppUnaryFunction
{
public:
  constexpr CppUnaryFunction() = default;

  constexpr CppUnaryFunction(CppUnaryFunctionPtr f_ptr) : f_ptr(f_ptr) {};

  double operator()(double a) const
  {
    return f_ptr(a);
  }

protected:
  CppUnaryFunctionPtr f_ptr = nullptr;
};

// we save the names along with the function pointers for convenience
// we could save only the function pointers, and the names only in the inventory
constexpr std::array<std::pair<std::string_view, CppUnaryFunction>, 31> builtin_unary_functions = {{
  {"cos",   CppUnaryFunction(std::cos)},
  {"sin",   CppUnaryFunction(std::sin)},
  {"tan",   CppUnaryFunction(std::tan)},

  {"acos",  CppUnaryFunction(std::acos)},
  {"asin",  CppUnaryFunction(std::asin)},
  {"atan",  CppUnaryFunction(std::atan)},

  {"cosh",  CppUnaryFunction(std::cosh)},
  {"sinh",  CppUnaryFunction(std::sinh)},
  {"tanh",  CppUnaryFunction(std::tanh)},

  {"ch",    CppUnaryFunction(std::cosh)},
  {"sh",    CppUnaryFunction(std::sinh)},
  {"th",    CppUnaryFunction(std::tanh)},

  {"acosh", CppUnaryFunction(std::acosh)},
  {"asinh", CppUnaryFunction(std::asinh)},
  {"atanh", CppUnaryFunction(std::atanh)},

  {"ach",   CppUnaryFunction(std::acosh)},
  {"ash",   CppUnaryFunction(std::asinh)},
  {"ath",   CppUnaryFunction(std::atanh)},

  {"sqrt",  CppUnaryFunction(std::sqrt)},
  {"log",   CppUnaryFunction(std::log10)},
  {"lg",    CppUnaryFunction(std::log2)},
  {"ln",    CppUnaryFunction(std::log)},
  {"abs",   CppUnaryFunction(std::abs)},
  {"exp",   CppUnaryFunction(std::exp)},
  {"floor", CppUnaryFunction(std::floor)},
  {"ceil",  CppUnaryFunction(std::ceil)},
  {"erf",   CppUnaryFunction(std::erf)},
  {"erfc",  CppUnaryFunction(std::erfc)},
  {"gamma", CppUnaryFunction(std::tgamma)},
  {"Γ",     CppUnaryFunction(std::tgamma)},
}};

}
