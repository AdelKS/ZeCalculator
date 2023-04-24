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

#include <zecalculator/function.h>
#include <zecalculator/mathworld.h>
#include <zecalculator/utils/evaluation.h>

namespace zc {

tl::expected<double, EvaluationError> Function::evaluate(const std::vector<double>& args,
                                                         const MathWorld& world) const
{
  if (not bool(tree)) [[unlikely]]
    return tl::unexpected(EvaluationError::invalid_function());
  else if (std::holds_alternative<std::monostate>(tree.value())) [[unlikely]]
    return tl::unexpected(EvaluationError::empty_expression());
  else if (args.size() != vars.size()) [[unlikely]]
    return tl::unexpected(EvaluationError::mismatched_fun_args());

  // make a keyword argument list out of the positional arguments
  // note: this overhead will be improved when we bind expressions to math worlds
  name_map<double> var_vals;
  for (size_t i = 0 ; i != vars.size() ; i++)
    var_vals[vars[i]] = args[i];

  return zc::evaluate(*tree, var_vals, world);
}

tl::expected<double, EvaluationError> Function::operator () (const std::vector<double>& args,
                                                             const MathWorld& world) const
{
  return evaluate(args, world);
}

} // namespace zc
