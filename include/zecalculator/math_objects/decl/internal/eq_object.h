#pragma once

#include <zecalculator/error.h>
#include <zecalculator/math_objects/object_list.h>
#include <zecalculator/parsing/data_structures/decl/ast.h>
#include <zecalculator/parsing/data_structures/deps.h>

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

  deps::Deps direct_dependencies() const;

  parsing::tokens::Text name = {};

  /// @brief full expression of the left hand side (function call or variable name)
  /// @note  does not include white spaces
  parsing::tokens::Text lhs = {};

  /// @brief name of the input variables
  std::vector<std::string> var_names = {};

  /// @brief equation
  std::string equation = {};

  /// @brief the right side to the equal sign in the equation
  parsing::AST rhs = {};
};

} // namespace internal
} // namespace zc
