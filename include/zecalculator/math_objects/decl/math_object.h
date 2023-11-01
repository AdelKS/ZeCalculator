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

#include <string>

#include <zecalculator/parsing/shared.h>

namespace zc {

template <parsing::Type type>
class MathWorld;

template <parsing::Type type>
class MathObject
{
public:
  const std::string& get_name() const;

protected:

  MathObject(const MathWorld<type>* world);
  MathObject(std::string name, const MathWorld<type>* world);

  void set_name(std::string name);

  std::string name;

  // non-owning pointer to the mathworld that contains this object
  const MathWorld<type>* mathworld;

  template <parsing::Type>
  friend class MathWorld;

};

}