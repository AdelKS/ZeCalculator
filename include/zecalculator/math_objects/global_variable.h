#pragma once

#include "zecalculator/parsing/parser.h"
#include <zecalculator/math_objects/decl/expression.h>

namespace zc {

/// @brief there's not mathematical difference between a global variable
///        and a simple mathematical expression. It just makes more sense
///        when we add one to a math world: a global variable is an expression
///        that has a name
template <parsing::Type type>
using GlobalVariable = Expression<type>;

namespace ast {
  using GlobalVariable = zc::Expression<parsing::AST>;
}

}
