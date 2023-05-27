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

#include <iostream>

using namespace zc;
using namespace tl;
using namespace std;

int main()
{
  MathWorld world;

  // Notes about adding a math object to a math world:
  // - Each added object exists only within the math world that creates it
  // - Adding a math object returns a tl::expected that can have an error instead of the
  //   handle to the function (e.g.: invalid format for the name, or name is already taken)

  // Add a function named "f", note that the constant "my_constant" is only defined after
  // Note: the .value() call from tl::expected<> throws if it actually hold an error
  auto f = world.add("f", Function({"x"}, "x + my_constant + cos(math::pi)")).value();

  // Add a global constant called "my_constant" with an initial value of 3.0
  // Note: the .value() call from tl::expected<> throws if it actually hold an error
  auto cst = world.add<GlobalConstant>("my_constant", 3.0).value();

  // Evaluate function and get the value
  // Notes:
  // - We know the expression is correct, otherwise the call `.value()` will throw
  // - The error can be recovered with '.error()`
  // - To know if the result is correct
  //   - call `.has_value()`
  //   - use the `bool()` operator on the expression
  std::cout << f({1}).value()  << std::endl; // == 3

  // overwrite the value of the global constant
  *cst = 5.0;

  // evaluate function again and get the new value
  std::cout << f({1}).value()  << std::endl; // == 5

  // change 'f': new variable names and count, call a function g
  *f = Function({"y", "z"}, "y + z + my_constant + g(y)");

  // define function 'g'
  auto g = world.add<Function>("g").value();

  // assign input variables and expression to 'g'
  *g = Function({"z"}, "2*z + my_constant");

  // evaluate function again and get the new value
  std::cout << f({3, 4}).value() << std::endl; // == 23

  return 0;
}
