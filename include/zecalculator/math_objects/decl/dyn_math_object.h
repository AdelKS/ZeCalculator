#pragma once

/****************************************************************************
**  Copyright (c) 2023, Adel Kara Slimane <adel.ks@zegrapher.com>
**
**  This file is part of ZeCalculator's source code.
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

#include <zecalculator/error.h>
#include <zecalculator/external/expected.h>
#include <zecalculator/math_objects/decl/math_object.h>
#include <zecalculator/math_objects/object_list.h>
#include <zecalculator/parsing/types.h>

namespace zc {

template <parsing::Type type>
class MathWorld;

namespace parsing {
  template <Type type>
  struct bind;
}

template <parsing::Type type>
using MathObjectsVariant = to_variant_t<MathObjects<type>>;

template <parsing::Type type>
struct DynMathObject
{
  template <class... DBL>
    requires (std::is_convertible_v<DBL, double> and ...)
  tl::expected<double, Error> operator () (DBL... val) const;

  template <class... DBL>
    requires (std::is_convertible_v<DBL, double> and ...)
  tl::expected<double, Error> evaluate(DBL... val) const;

  MathObjectsVariant<type> variant;
  MathWorldObjectHandle<type> handle;
};

} // namespace zc
