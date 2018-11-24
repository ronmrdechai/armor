// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#include "assoc.h"

template <typename T>
struct tst_common : test::assoc_test<T>, testing::Test {};
TYPED_TEST_CASE(tst_common, tst_common_types);

TYPED_TEST(tst_common, insert_hint_wrong_hint) {
    TypeParam t{ TestFixture::key_to_value("bar") };
    auto hint = t.find("bar");

    auto it = t.insert(hint, TestFixture::key_to_value("foobar"));
    EXPECT_NE(t.end(), t.find("baobar"));
    EXPECT_EQ("foobar", TestFixture::value_to_key(*it));
}

TYPED_TEST(tst_common, emplace_hint_wrong_hint) {
    TypeParam t{ TestFixture::key_to_value("bar") };
    auto hint = t.find("bar");

    auto it = t.emplace_hint(hint, TestFixture::key_to_value("foobar"));
    EXPECT_NE(t.end(), t.find("baobar"));
    EXPECT_EQ("foobar", TestFixture::value_to_key(*it));
}

TYPED_TEST(tst_common, key_comp) {
    EXPECT_TRUE(TestFixture::has_key_compare);

    TypeParam t;
    EXPECT_TRUE((std::is_same_v<decltype(t.key_comp()), typename TypeParam::key_compare>));
}

using tst = rmr::detail::ternary_search_tree<int, std::less<char>, std::string, std::allocator<int>>;

TEST(tst, reverse_iteration) {
    tst t;

    t.emplace(t.root(), "foo", 42);
    t.emplace(t.root(), "baz", 0);
    t.emplace(t.root(), "bar", 44);

    t.erase(t.find("foo"));
    t.erase(t.find("bar"));

    t.emplace(t.root(), "foo",     1);
    t.emplace(t.root(), "fooqux",  5);
    t.emplace(t.root(), "foobaz",  3);
    t.emplace(t.root(), "fooquux", 4);
    t.emplace(t.root(), "foobar",  2);

    EXPECT_EQ(6u, std::distance(t.rbegin(), t.rend()));

    rmr::detail::write_dot(t.croot(), std::ofstream("broken_tst.dot"));
    for (auto it = t.rbegin(); it != t.rend(); ++it)
        std::cout << *it << std::endl;
}
