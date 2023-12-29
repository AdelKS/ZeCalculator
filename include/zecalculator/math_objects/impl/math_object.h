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

#include <zecalculator/math_objects/decl/math_object.h>

namespace zc {

template <parsing::Type type>
MathObject<type>::MathObject(size_t slot, MathWorld<type>* world)
  : slot(slot), mathworld(world)
{}

template <parsing::Type type>
MathObject<type>::MathObject(std::string name, size_t slot, const MathWorld<type>* world)
  : name(std::move(name)), slot(slot), mathworld(world)
{}

template <parsing::Type type>
void MathObject<type>::set_name(std::string name)
{
  this->name = std::move(name);
}

template <parsing::Type type>
const std::string& MathObject<type>::get_name() const
{
  return name;
}

}
