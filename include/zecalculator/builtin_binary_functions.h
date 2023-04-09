#pragma once

#include <string_view>
#include <array>
#include <cmath>

namespace zc {

using CppBinaryFunctionPtr = double (*) (double, double);

class CppBinaryFunction
{
public:
  constexpr CppBinaryFunction(CppBinaryFunctionPtr f_ptr) : f_ptr(f_ptr) {};

  double operator()(double a, double b) const
  {
    return f_ptr(a ,b);
  }

protected:
  CppBinaryFunctionPtr f_ptr;
};

double plus(const double a, const double b);
double minus(const double a, const double b);
double multiply(const double a, const double b);
double divide(const double a, const double b);


// we save the names along with the function pointers for convenience
// we could save only the function pointers, and the names only in the inventory
constexpr std::array<std::pair<std::string_view, CppBinaryFunction>, 5> builtin_binary_functions = {{
  {"+", plus},
  {"-", minus},
  {"*", multiply},
  {"/", divide},
  {"^", CppBinaryFunction(std::pow)},
}};

}
