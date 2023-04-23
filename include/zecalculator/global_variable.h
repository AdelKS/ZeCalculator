#pragma once

#include <zecalculator/expression.h>

namespace zc {

/// @brief there's not mathematical difference between a global variable
///        and a simple mathematical expression. It just makes more sense
///        when we add one to a math world: a global variable is an expression
///        that has a name
using GlobalVariable = Expression;

}
