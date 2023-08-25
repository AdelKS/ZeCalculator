#pragma once

/****************************************************************************
**  Copyright (c) 2023, Adel Kara Slimane <adel.ks@zegrapher.com>
**
**  This file is part of ZeGrapher's source code.
**
**  ZeGrapher is free software: you may copy, redistribute and/or modify it
**  under the terms of the GNU General Public License as published by the
**  Free Software Foundation, either version 3 of the License, or (at your
**  option) any later version.
**
**  This file is distributed in the hope that it will be useful, but
**  WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include <zecalculator/math_objects/decl/function.h>

#include <zecalculator/evaluation/ast/evaluation.h>

namespace zc {

inline Function::Function(std::vector<std::string> input_vars, std::string expr)
{
  set_input_vars(std::move(input_vars));
  set_expression(std::move(expr));
}


inline void Function::set_input_vars(std::vector<std::string> input_vars)
{
  auto it = std::ranges::find_if_not(input_vars, parsing::is_valid_name);
  if (it != input_vars.end())
    vars = tl::unexpected(InvalidInputVar{*it});
  else vars = std::move(input_vars);
}

inline void Function::set_expression(std::string expr)
{
  // do nothing if it's the same expression
  if (expression == expr)
    return;

  expression = std::move(expr);

  if (expression.empty())
    tree = std::monostate();
  else
  {
    // workaround limitation in tl::expected when using and_then to implicitly converted-to types
    const auto parsing = parsing::tokenize(expression);
    if (parsing)
      tree = make_tree(parsing.value());
    else tree = tl::unexpected(parsing.error());
  }

}

inline std::optional<size_t> Function::argument_size() const
{
  if (vars)
    return vars->size();
  else return {};
}

inline Function::operator bool () const
{
  return bool(tree) and (not std::holds_alternative<std::monostate>(tree.value())) and bool(vars);
}

inline std::variant<Ok, Empty, parsing::Error> Function::parsing_status() const
{
  if (not tree.has_value())
    return tree.error();
  else if (std::holds_alternative<std::monostate>(tree.value()))
    return Empty();
  else return Ok();
}

inline const tl::expected<ast::Tree, parsing::Error>& Function::get_tree() const { return tree; }


inline tl::expected<double, eval::Error> Function::evaluate(const std::vector<double>& args,
                                                            const MathWorld& world,
                                                            size_t current_recursion_depth) const
{
  if (not bool(*this)) [[unlikely]]
    return tl::unexpected(eval::Error::invalid_function());
  else if (std::holds_alternative<std::monostate>(tree.value())) [[unlikely]]
    return tl::unexpected(eval::Error::empty_expression());
  else if (args.size() != vars->size()) [[unlikely]]
    return tl::unexpected(eval::Error::mismatched_fun_args());

  // make a keyword argument list out of the positional arguments
  // note: this overhead will be improved when we bind expressions to math worlds
  name_map<double> var_vals;
  for (size_t i = 0 ; i != vars->size() ; i++)
    var_vals[(*vars)[i]] = args[i];

  return zc::evaluate(*tree, var_vals, world, current_recursion_depth);
}

inline tl::expected<double, eval::Error> Function::evaluate(const std::vector<double>& args,
                                                            const MathWorld& world) const
{
  // this function is user called, so the recursion depth is zero
  return evaluate(args, world, 0);
}

inline tl::expected<double, eval::Error> Function::operator()(const std::vector<double>& args,
                                                              const MathWorld& world) const
{
  return evaluate(args, world, 0);
}

} // namespace zc
