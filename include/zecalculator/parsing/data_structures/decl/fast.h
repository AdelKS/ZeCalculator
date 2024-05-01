#pragma once

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

#include <zecalculator/parsing/data_structures/decl/shared.h>
#include <zecalculator/math_objects/forward_declares.h>
#include <zecalculator/parsing/types.h>
#include <zecalculator/utils/non_unique_ptr.h>
#include <zecalculator/utils/utils.h>

namespace zc {
namespace parsing {

  /// @brief A tree representation in an AST or RPN world
  /// @note when the math world is RPN based, this AST is simply an intermediate form
  ///       before being transformed into an RPN representation
  template <parsing::Type world_type>
  struct FAST
  {
    shared::Node<world_type> node;
    std::vector<FAST> subnodes = {};

    bool operator == (const FAST&) const;
  };

  } // namespace parsing
} // namespace zc
