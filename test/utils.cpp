#include <zecalculator/evaluation/object_cache.h>
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

  "ObjectCache test"_test = []()
  {
    eval::ObjectCache cache({1., 2., 3.}, {1., 2., 3.}, 4);
    cache.insert(4., 4.);
    cache.insert(4., 5.);
    cache.insert(2.5, 6.);
    cache.insert(0.5, 7.);
    cache.insert(3., 8.);
    cache.insert(4., 9.);
    cache.insert(0., 10.);

    std::vector vals = cache.get_cache().values();
    std::ranges::sort(vals);

    expect(vals == std::vector{7., 8., 9., 10.}) << cache.get_cache().values();
    expect(cache.get_cache().keys() == std::vector{0., 0.5, 3., 4.}) << cache.get_cache().keys();

    cache.set_buffer_size(6);
    cache.insert(-1., 11.);
    cache.insert(3.5, 12.);

    vals = cache.get_cache().values();
    std::ranges::sort(vals);

    expect(vals == std::vector{7., 8., 9., 10., 11., 12.}) << cache.get_cache().values();
    expect(cache.get_cache().keys() == std::vector{-1., 0., 0.5, 3., 3.5, 4.}) << cache.get_cache().keys();

    cache.set_buffer_size(3);

    vals = cache.get_cache().values();
    std::ranges::sort(vals);

    expect(vals == std::vector{10., 11., 12.}) << cache.get_cache().values();
    expect(cache.get_cache().keys() == std::vector{-1., 0., 3.5}) << cache.get_cache().keys();
  };

  "LHS parsing"_test = []()
  {
    using parsing::tokens::Text;

    std::string_view expr = "   func(x   , w, y  )  ";
    auto exp_lhs = parsing::parse_lhs(expr, expr);
    expect(bool(exp_lhs)) << fatal;
    expect(*exp_lhs
           == parsing::LHS{
             .name = Text{"func", 3},
             .input_vars = {Text{"x", 8}, Text{"w", 14}, Text{"y", 17}},
             .substr = Text{"func(x   , w, y  )", 3},
           });
  };
}
