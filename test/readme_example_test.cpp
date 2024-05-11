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
#include <zecalculator/test-utils/print-utils.h>

using namespace zc;
using namespace tl;
using namespace std;

int main()
{
  rpn::MathWorld world;

  // Notes about adding a math object to a math world:
  // - Each added object exists only within the math world that creates it
  // - Adding a math object returns a DynMathObject reference that is essentially an expected<variant, error>
  //   with some helper functions.
  //   - the variant contains all the possible objects: function, sequence, global constant, global variable, cpp function
  //   - the error expresses what went wrong in parsing the equation

  // Add a one parameter function named "f"
  // Note that 'my_constant' is only defined later
  // - this function's state will be updated once 'my_constant' is defined
  // - (re)defining objects within a math world can potentially modify every other objects
  rpn::DynMathObject& obj1 = world.add("f(x) = x + my_constant + cos(math::pi)");

  // We can query the direct dependencies of any object
  // but only Function and Sequence instances with a valid equation return a non-empty set
  assert(bool(world.direct_dependencies(obj1)
              == deps::Deps{{"my_constant", {deps::Dep::VARIABLE, {11}}},
                            {"cos", {deps::Dep::FUNCTION, {25}}},
                            {"math::pi", {deps::Dep::VARIABLE, {29}}}}));

  // the expected should hold an error since 'my_constant' is undefined at this point
  assert(not obj1.has_value()
         and obj1.error()
               == Error::undefined_variable(parsing::tokens::Text{"my_constant", 11},
                                            "f(x) = x + my_constant + cos(math::pi)"));

  // Add a global constant called "my_constant" with an initial value of 3.0
  rpn::DynMathObject& obj2 = world.add("my_constant = 3.0");

  // now that 'my_constant' is defined, 'obj1' gets modified to properly hold a function
  // Note that defining an object in the MathWorld may affect any other object
  // -> Redefining objects is NOT thread-safe
  assert(obj1.holds<rpn::Function>());

  // We can evaluate 'obj1'
  // note: we could also do it when 'my_constant' was undefined,
  //       in that case the result would be the same error as above
  expected<double, Error> eval = obj1(1.0);

  // Notes:
  // - We know the expression is correct, otherwise the call `.value()` will throw
  // - The error can be recovered with '.error()`
  // - To know if the result is correct
  //   - call `.has_value()`
  //   - use the `bool()` operator on 'eval'
  assert(eval.value() == 3);

  // add a single argument function 'g' to the world
  world.add("g(z) = 2*z + my_constant");

  // redefine what 'obj1' using a new equation
  // - Now it's the Fibonacci sequence called 'u'
  world.redefine(obj1, "u(n) = 0 ; 1 ; u(n-1) + u(n-2)");

  // should hold a Sequence now
  assert(obj1.holds<rpn::Sequence>());

  // evaluate function again and get the new value
  assert(obj1(10).value() == 55);

  // ======================================================================================

  // the underlying objects can be retrieved either by using the fact that
  // DynMathObject publicly inherits expected<variant, error> or the 'value_as' helper function:
  // - "value" prefix just like expected::value, i.e. can throw
  // - "value_as" as a wrapper to std::get<>(expected::value), can throw for two different reasons
  //   - the expected has an error
  //   - the alternative asked is not the actual one held by the variant
  [[maybe_unused]] rpn::Sequence& u = obj1.value_as<rpn::Sequence>();
  [[maybe_unused]] GlobalConstant& my_constant = obj2.value_as<GlobalConstant>();

  // each specific math object has extra public methods that may prove useful

  return 0;
}
