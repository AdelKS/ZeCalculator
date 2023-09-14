/****************************************************************************
**  Copyright (c) 2023, Adel Kara Slimane <adel.ks@zegrapher.com>
**
**  This file is part of ZeCalculator.
**
**  ZeCalculators is free software: you may copy, redistribute and/or modify it
**  under the terms of the GNU Affero General Public License as published by the
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

#pragma once

#include <optional>
#include <span>
#include <vector>

#include <zecalculator/parsing/data_structures/token.h>
#include <zecalculator/utils/utils.h>

namespace zc {
namespace ast {
namespace node {

struct Function;
struct InputVariable;
using Variable = parsing::tokens::Variable;
using Number = parsing::tokens::Number;

}

using Tree = std::variant<std::monostate, node::Function, node::InputVariable, node::Variable, node::Number>;


parsing::tokens::Text text_token(const ast::Tree& token);

SubstrInfo substr_info(const ast::Tree& token);

} // namespace ast
} // namespace zc
