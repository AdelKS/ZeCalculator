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

#include <zecalculator/zecalculator.h>

using namespace zc;
using namespace tl;
using namespace std;

int main()
{
  ast::MathWorld world;

  // Notes about adding a math object to a math world:
  // - Each added object exists only within the math world that creates it
  // - Adding a math object returns a tl::expected that can have an error instead of a
  //   ref to the object (e.g.: invalid format for the name, or name is already taken)

  // Add a global constant called "my_constant" with an initial value of 3.0
  // Note: the .value() call from tl::expected<> throws if it actually holds an error
  GlobalConstant& my_constant = world.add<GlobalConstant>("my_constant", 3.0).value();

  // Add a one parameter function named "f"
  ast::Function<1>& f = world.add<ast::Function<1>>("f", Vars<1>{"x"}, "x + my_constant + cos(math::pi)").value();

  // We know the expression is correct
  assert(not f.error());

  // Evaluate function, returns an 'expected'
  expected<double, Error> eval = f({1});

  // Notes:
  // - We know the expression is correct, otherwise the call `.value()` will throw
  // - The error can be recovered with '.error()`
  // - To know if the result is correct
  //   - call `.has_value()`
  //   - use the `bool()` operator on the expression
  assert(eval.value() == 3);

  // overwrite the value of the global constant
  my_constant = 5.0;

  // evaluate function again and get the new value
  assert(f({1}).value() == 5);

  // define one parameter function 'g'
  world.add<ast::Function<1>>("g", Vars<1>{"z"}, "2*z + my_constant").value();

  // change 'f':
  // - different number of input variables and different names
  // - expression calls a function g
  f.set(Vars<1>{"y"}, "y + my_constant + g(y)");

  // evaluate function again and get the new value
  assert(f({3}).value() == 19);

  return 0;
}
