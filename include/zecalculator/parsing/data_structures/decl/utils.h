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

#include <zecalculator/parsing/types.h>
#include <zecalculator/utils/utils.h>
#include <zecalculator/parsing/data_structures/decl/fast.h>
#include <zecalculator/parsing/data_structures/decl/rpn.h>

namespace zc {
namespace parsing {

template <parsing::Type type>
using Parsing = std::conditional_t<type == parsing::Type::FAST, FAST<parsing::Type::FAST>, RPN>;

template <parsing::Type type>
struct LinkedFunc {
  Parsing<type> repr;
  size_t args_num;
};

template <parsing::Type type>
struct LinkedSeq
{
  std::vector<Parsing<type>> repr;
  size_t slot;
};

template <parsing::Type type>
struct LinkedData
{
  std::vector<tl::expected<Parsing<type>, zc::Error>> repr;
  size_t slot;
};

} // namespace parsing
} // namespace zc
