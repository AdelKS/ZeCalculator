#include <zecalculator/zecalculator.h>

// testing specific headers
#include <boost/ut.hpp>
#include <zecalculator/test-utils/print-utils.h>
#include <zecalculator/test-utils/structs.h>

using namespace zc;

int main()
{
  using namespace boost::ut;

  "SlottedDeque push in specific slot"_test = []()
  {
    SlottedDeque<size_t> sdeque;

    // pushing in slot/index == 2
    sdeque.push(42, 2);

    // the next free slots should be 0, 1, 3
    expect(sdeque.push(42) == 0);
    expect(sdeque.push(42) == 1);
    expect(sdeque.push(42) == 3);
  };

}
