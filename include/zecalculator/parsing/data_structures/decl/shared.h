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

#include <zecalculator/math_objects/forward_declares.h>
#include <zecalculator/parsing/data_structures/token.h>
#include <zecalculator/parsing/types.h>
#include <zecalculator/utils/utils.h>

namespace zc {

namespace parsing {

template <parsing::Type type>
struct LinkedFunc;

template <parsing::Type type>
struct LinkedSeq;

template <parsing::Type type>
struct LinkedData;

  namespace shared {
    namespace node {

      struct Add {
        bool operator == (const Add&) const = default;
      };
      struct Subtract {
        bool operator == (const Subtract&) const = default;
      };
      struct Multiply {
        bool operator == (const Multiply&) const = default;
      };
      struct Divide {
        bool operator == (const Divide&) const = default;
      };
      struct Power {
        bool operator == (const Power&) const = default;
      };

      struct UnaryMinus {
        bool operator == (const UnaryMinus&) const = default;
      };

      struct Number {
        double value;

        bool operator == (const Number&) const = default;
      };

      struct InputVariable {
        size_t index;

        bool operator == (const InputVariable&) const = default;
      };
    } // namespace node

    template <parsing::Type world_type>
    using Node = std::variant<node::Add,
                              node::Subtract,
                              node::Multiply,
                              node::Divide,
                              node::Power,
                              node::UnaryMinus,
                              node::Number,
                              node::InputVariable,
                              CppFunction<1>,
                              CppFunction<2>,
                              const double *,
                              const LinkedFunc<world_type> *,
                              const LinkedSeq<world_type> *,
                              const LinkedData<world_type> *>;

  } // namespace shared

} // namespace parsing
} // namespace zc
