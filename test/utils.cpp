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
    eval::ObjectCache cache({1., 2., 3.}, {1., 2., 3.}, 0, 4);
    cache.insert(0, 4., 4.);
    cache.insert(0, 4., 5.);
    cache.insert(0, 2.5, 6.);
    cache.insert(0, 0.5, 7.);
    cache.insert(0, 3., 8.);
    cache.insert(0, 4., 9.);
    cache.insert(0, 0., 10.);

    std::vector vals = cache.get_cache().values();
    std::ranges::sort(vals);

    expect(vals == std::vector{7., 8., 9., 10.}) << cache.get_cache().values();
    expect(cache.get_cache().keys() == std::vector{0., 0.5, 3., 4.}) << cache.get_cache().keys();

    cache.set_buffer_size(6);
    cache.insert(0, -1., 11.);
    cache.insert(0, 3.5, 12.);

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

  "ObjectCache: update existing key in full cache preserves siblings"_test = []()
  {
    eval::ObjectCache cache({1., 2., 3., 4.}, {1., 2., 3., 4.}, 0, 4);

    // re-inserting an existing key in a full cache should refresh the value
    // and the age, but must NOT evict any other entry
    cache.insert(0, 2., 20.);

    expect(cache.get_cache().keys() == std::vector{1., 2., 3., 4.})
      << cache.get_cache().keys();
    expect(cache.get_cache().values() == std::vector{1., 20., 3., 4.})
      << cache.get_cache().values();

    // 1 is the oldest now; the next *new* key should evict it
    cache.insert(0, 5., 50.);
    expect(cache.get_cache().keys() == std::vector{2., 3., 4., 5.})
      << cache.get_cache().keys();

    // and re-inserting 2 (which was just refreshed) again should leave size==4
    cache.insert(0, 2., 200.);
    expect(cache.get_cache().keys() == std::vector{2., 3., 4., 5.})
      << cache.get_cache().keys();
    expect(cache.get_value(0, 2.) == std::optional{200.});
  };

  "ObjectCache: revision change wipes prior entries"_test = []()
  {
    eval::ObjectCache cache({1., 2., 3.}, {10., 20., 30.}, 0, 4);

    expect(cache.get_value(0, 2.) == std::optional{20.});
    expect(not cache.get_value(1, 2.).has_value());

    cache.insert(1, 5., 50.);

    expect(cache.get_cache().keys() == std::vector{5.}) << cache.get_cache().keys();
    expect(cache.get_value(1, 5.) == std::optional{50.});
    expect(not cache.get_value(0, 1.).has_value());
    expect(cache.get_cached_revision() == 1_u);
  };

  "ObjectCache: shrink buffer to zero clears the cache"_test = []()
  {
    eval::ObjectCache cache({1., 2., 3.}, {1., 2., 3.}, 0, 4);

    cache.set_buffer_size(0);

    expect(cache.get_cache().keys().empty()) << cache.get_cache().keys();
    expect(cache.get_buffer_size() == 0_u);
  };

  "ObjectCache: buffer of size 1 keeps only newest"_test = []()
  {
    eval::ObjectCache cache(1);
    cache.insert(0, 1., 10.);
    cache.insert(0, 2., 20.);
    cache.insert(0, 3., 30.);

    expect(cache.get_cache().keys() == std::vector{3.}) << cache.get_cache().keys();
    expect(cache.get_cache().values() == std::vector{30.}) << cache.get_cache().values();

    // re-inserting the same key in a size-1 cache should be a value update,
    // not a clear-and-reinsert
    cache.insert(0, 3., 300.);
    expect(cache.get_cache().keys() == std::vector{3.}) << cache.get_cache().keys();
    expect(cache.get_cache().values() == std::vector{300.}) << cache.get_cache().values();
  };

  "ObjectCache: shrink to a non-empty size keeps the newest entries"_test = []()
  {
    eval::ObjectCache cache(4);
    cache.insert(0, 1., 1.); // oldest
    cache.insert(0, 2., 2.);
    cache.insert(0, 3., 3.);
    cache.insert(0, 4., 4.); // newest

    cache.set_buffer_size(2);

    // only the two newest entries (3 and 4) should remain
    expect(cache.get_cache().keys() == std::vector{3., 4.}) << cache.get_cache().keys();
    expect(cache.get_cache().values() == std::vector{3., 4.}) << cache.get_cache().values();
    expect(cache.get_buffer_size() == 2_u);
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
