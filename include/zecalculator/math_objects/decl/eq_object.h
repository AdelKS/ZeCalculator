#pragma once

#include <zecalculator/parsing/data_structures/decl/ast.h>

#include <string>

namespace zc {

/// @brief internal representation for objects defined through an equation
struct EqObject
{
  enum Category {AUTO, FUNCTION, SEQUENCE, GLOBAL_CONSTANT};
  Category cat;

  /// @brief equation
  std::string equation = {};

  std::string name = {};

  /// @brief the left side to the equal sign in the equation
  parsing::AST lhs = {};

  /// @brief the right side to the equal sign in the equation
  parsing::AST rhs = {};
};

} // namespace zc
