#pragma once

#include <zecalculator/evaluation/error.h>
#include <zecalculator/math_objects/builtin_binary_functions.h>
#include <zecalculator/math_objects/builtin_unary_functions.h>
#include <zecalculator/math_objects/decl/function.h>
#include <zecalculator/math_objects/global_constant.h>
#include <zecalculator/mathworld/decl/mathworld.h>
#include <zecalculator/utils/name_map.h>

namespace zc {

namespace eval {

struct Node
{
  const ast::MathWorld& world;
  const name_map<double>& input_vars;
  const size_t current_recursion_depth = 0;


  using ReturnType = tl::expected<double, Error>;

  // using T = std::remove_cvref_t<decltype(node)>;

  ReturnType operator () (std::monostate);

  ReturnType operator () (const ast::node::Function& node);

  ReturnType operator () (const ast::node::Variable& node);

  ReturnType operator () (const ast::node::Number& node);

};

}

/// @brief evaluates a syntax tree using a given math world
/// @param tree: tree to evaluate
/// @param input_vars: variables that are given as input to the tree, will shadow any variable in the math world
/// @param world: math world (contains functions, global constants... etc)
inline tl::expected<double, eval::Error> evaluate(const ast::Tree& tree,
                                                  const name_map<double>& input_vars,
                                                  const ast::MathWorld& world,
                                                  size_t current_recursion_depth);

/// @brief evaluates a syntax tree using a given math world
inline tl::expected<double, eval::Error> evaluate(const ast::Tree& tree,
                                                  const name_map<double>& input_vars,
                                                  const ast::MathWorld& world);

/// @brief evaluates a syntax tree using a given math world
inline tl::expected<double, eval::Error> evaluate(const ast::Tree& tree, const ast::MathWorld& world);


}
