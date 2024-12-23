#pragma once

#include <zecalculator/error.h>
#include <zecalculator/math_objects/object_list.h>
#include <zecalculator/parsing/data_structures/decl/ast.h>

#include <string>

namespace zc {

template <parsing::Type type>
class MathWorld;

namespace internal {

/// @brief internal representation for objects defined through an equation
struct EqObject
{
  enum Category {AUTO, FUNCTION, SEQUENCE, GLOBAL_CONSTANT};
  Category cat;

  template <parsing::Type type>
  tl::expected<MathObjectsVariant<type>, Error> to_expected_unbound() const;

  template <parsing::Type type>
  tl::expected<MathObjectsVariant<type>, Error> to_expected(const zc::MathWorld<type>& mathworld) const;

  /// @brief equation
  std::string equation = {};

  std::string name = {};

  /// @brief the left side to the equal sign in the equation
  parsing::AST lhs = {};

  /// @brief the right side to the equal sign in the equation
  parsing::AST rhs = {};
};

} // namespace internal
} // namespace zc
