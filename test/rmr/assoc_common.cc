// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#include <gtest/gtest.h>

#include "assoc.h"

template <typename T> struct assoc_common : testing::Test {
    T less_trie    = test::less_trie<T>;
    T greater_trie = test::greater_trie<T>;
    T roman_trie   = test::roman_trie<T>;

    static typename T::value_type
    key_to_value(const typename T::key_type& k) { return test::key_to_value<T>(k); }
    static typename T::key_type
    value_to_key(const typename T::value_type& v) { return test::value_to_key<T>(v); }
    template <typename... Keys>
    static T make_container(Keys&&... keys) { return test::make_container<T>(std::forward<Keys>(keys)...); }

    static void assert_empty(const T& t) { test::assert_empty<T>(t); }
};
TYPED_TEST_CASE(assoc_common, assoc_common_types);

TYPED_TEST(assoc_common, default_empty_returns_true) {
    TypeParam t;
    EXPECT_TRUE(t.empty());
}

TYPED_TEST(assoc_common, default_size_is_0) {
    TypeParam t;
    EXPECT_EQ(0u, t.size());
}

TYPED_TEST(assoc_common, default_distance_is_0) {
    TypeParam t;
    EXPECT_EQ(0u, std::distance(t.begin(), t.end()));
}

TYPED_TEST(assoc_common, roman_trie_size_is_7) {
    TypeParam t = TestFixture::roman_trie;
    EXPECT_EQ(7u, t.size());
}

TYPED_TEST(assoc_common, roman_trie_distance_is_7) {
    TypeParam t = TestFixture::roman_trie;
    EXPECT_EQ(7u, std::distance(t.begin(), t.end()));
}

TYPED_TEST(assoc_common, copy_constructor) {
    TypeParam t = TestFixture::roman_trie;
    TypeParam s(t);

    EXPECT_EQ(t, s);
}

TYPED_TEST(assoc_common, change_original_after_copy_construction) {
    TypeParam t = TestFixture::roman_trie;
    TypeParam s(t);

    t.erase("romulus");
    EXPECT_NE(t, s);
}

TYPED_TEST(assoc_common, change_copy_after_copy_construction) {
    TypeParam t = TestFixture::roman_trie;
    TypeParam s(t);

    s.erase("romulus");
    EXPECT_NE(t, s);
}

TYPED_TEST(assoc_common, move_constructor) {
    TypeParam t = TestFixture::roman_trie;
    TypeParam s(std::move(t));

    EXPECT_EQ(TestFixture::roman_trie, s);
    EXPECT_TRUE(t.empty());
}

TYPED_TEST(assoc_common, iterator_constructor) {
    std::vector<typename TypeParam::value_type> v {
        TestFixture::key_to_value("bar"),
        TestFixture::key_to_value("baz"),
        TestFixture::key_to_value("foo")
    };

    TypeParam t(v.begin(), v.end());
    EXPECT_TRUE(std::equal(t.begin(), t.end(), v.begin()));
}

TYPED_TEST(assoc_common, initializer_list_constructor) {
    std::vector<typename TypeParam::value_type> v {
        TestFixture::key_to_value("bar"),
        TestFixture::key_to_value("baz"),
        TestFixture::key_to_value("foo")
    };

    TypeParam t{
        TestFixture::key_to_value("bar"),
        TestFixture::key_to_value("baz"),
        TestFixture::key_to_value("foo")
    };
    EXPECT_TRUE(std::equal(t.begin(), t.end(), v.begin()));
}

TYPED_TEST(assoc_common, copy_assignment) {
    TypeParam t = TestFixture::roman_trie;
    TypeParam s;
    s = t;

    EXPECT_EQ(t, s);
}

TYPED_TEST(assoc_common, change_original_after_copy_assignment) {
    TypeParam t = TestFixture::roman_trie;
    TypeParam s;
    s = t;

    t.erase("romulus");
    EXPECT_NE(t, s);
}

TYPED_TEST(assoc_common, change_copy_after_copy_assignment) {
    TypeParam t = TestFixture::roman_trie;
    TypeParam s;
    s = t;

    s.erase("romulus");
    EXPECT_NE(t, s);
}

TYPED_TEST(assoc_common, old_data_gone_after_copy_assignment) {
    TypeParam t = TestFixture::roman_trie;
    TypeParam s{ TestFixture::key_to_value("foo") };
    s = t;

    EXPECT_EQ(s.end(), s.find("foo"));
}

TYPED_TEST(assoc_common, move_assignment) {
    TypeParam t = TestFixture::roman_trie;
    TypeParam s;
    s = std::move(t);

    EXPECT_EQ(TestFixture::roman_trie, s);
    EXPECT_TRUE(t.empty());
}

TYPED_TEST(assoc_common, old_data_gone_after_move_assignment) {
    TypeParam t = TestFixture::roman_trie;
    TypeParam s{ TestFixture::key_to_value("foo") };
    s = std::move(t);

    EXPECT_EQ(s.end(), s.find("foo"));
}

TYPED_TEST(assoc_common, empty_after_clear) {
    TypeParam t = TestFixture::roman_trie;
    t.clear();
    TestFixture::assert_empty(t);
}

TYPED_TEST(assoc_common, empty_after_move_construct) {
    TypeParam t = TestFixture::roman_trie;
    TypeParam(std::move(t));

    TestFixture::assert_empty(t);
}

TYPED_TEST(assoc_common, empty_after_move_assign) {
    TypeParam t = TestFixture::roman_trie;
    TypeParam{} = std::move(t);

    TestFixture::assert_empty(t);
}

TYPED_TEST(assoc_common, iteration_is_sorted) {
    TypeParam t = TestFixture::roman_trie;

    std::vector<typename TypeParam::key_type> unsorted;
    std::transform(t.begin(), t.end(), std::back_inserter(unsorted), TestFixture::value_to_key);

    std::vector<typename TypeParam::key_type> sorted = unsorted;
    std::sort(sorted.begin(), sorted.end());

    EXPECT_EQ(unsorted, sorted);
}

TYPED_TEST(assoc_common, reverse_iteration_is_reverse_sorted) {
    TypeParam t = TestFixture::roman_trie;

    std::vector<typename TypeParam::key_type> unsorted;
    std::transform(t.rbegin(), t.rend(), std::back_inserter(unsorted), TestFixture::value_to_key);

    std::vector<typename TypeParam::key_type> sorted = unsorted;
    std::sort(sorted.begin(), sorted.end());
    std::reverse(sorted.begin(), sorted.end());

    EXPECT_EQ(unsorted, sorted);
}

TYPED_TEST(assoc_common, reverse_iteration_covers_whole_container) {
    TypeParam t = TestFixture::roman_trie;
    EXPECT_EQ(std::distance(t.begin(), t.end()), std::distance(t.rbegin(), t.rend()));
}

TYPED_TEST(assoc_common, reverse_iteration_is_reversed) {
    TypeParam t = TestFixture::roman_trie;

    std::vector<typename TypeParam::key_type> not_reversed;
    std::transform(t.begin(), t.end(), std::back_inserter(not_reversed), TestFixture::value_to_key);

    std::vector<typename TypeParam::key_type> reversed;
    std::transform(t.rbegin(), t.rend(), std::back_inserter(reversed), TestFixture::value_to_key);

    std::reverse(not_reversed.begin(), not_reversed.end());

    EXPECT_EQ(not_reversed, reversed);
}

TYPED_TEST(assoc_common, max_size_is_uint64_max) {
    TypeParam t;
    EXPECT_EQ(UINT64_MAX, t.max_size());
}

TYPED_TEST(assoc_common, insert_size_change) {
    TypeParam t;
    t.insert( TestFixture::key_to_value("foo") );
    EXPECT_EQ(1u, t.size());
    t.insert( TestFixture::key_to_value("bar") );
    EXPECT_EQ(2u, t.size());
}

TYPED_TEST(assoc_common, insert_existing_size_no_change) {
    TypeParam t;
    t.insert( TestFixture::key_to_value("foo") );
    EXPECT_EQ(1u, t.size());
    t.insert( TestFixture::key_to_value("foo") );
    EXPECT_EQ(1u, t.size());
}

TYPED_TEST(assoc_common, insert_return_value) {
    TypeParam t;
    auto [it, inserted] = t.insert( TestFixture::key_to_value("foo") );

    EXPECT_TRUE(inserted);
    ASSERT_NE(t.end(), it);
    EXPECT_EQ("foo", TestFixture::value_to_key(*it));
}

TYPED_TEST(assoc_common, insert_existing_return_value) {
    TypeParam t;

    t.insert( TestFixture::key_to_value("foo") );

    auto [it, inserted] = t.insert( TestFixture::key_to_value("foo") );

    EXPECT_FALSE(inserted);
    ASSERT_NE(t.end(), it);
    EXPECT_EQ("foo", TestFixture::value_to_key(*it));
}

TYPED_TEST(assoc_common, insert_lvalue_reference) {
    TypeParam t;

    typename TypeParam::value_type v = TestFixture::key_to_value("foo");
    t.insert(v);
    EXPECT_EQ(1u, t.size());
}

TYPED_TEST(assoc_common, insert_range_size_increase) {
    TypeParam t;

    std::vector<typename TypeParam::value_type> v{
        TestFixture::key_to_value("foo"),
        TestFixture::key_to_value("bar"),
        TestFixture::key_to_value("baz")
    };
    t.insert(v.begin(), v.end());

    EXPECT_EQ(3u, t.size());
}

TYPED_TEST(assoc_common, insert_initializer_list_size_increase) {
    TypeParam t;

    t.insert({
        TestFixture::key_to_value("foo"),
        TestFixture::key_to_value("bar"),
        TestFixture::key_to_value("baz")
    });

    EXPECT_EQ(3u, t.size());
}

TYPED_TEST(assoc_common, prefixed_with) {
    std::vector<std::string> v{ "rubens", "ruber", "rubicon", "rubicundus" };
    TypeParam t = TestFixture::roman_trie;

    auto [first, last] = t.prefixed_with("rub");

    EXPECT_EQ(4u, std::distance(first, last));
    std::size_t i = 0;
    for (auto it = first; it != last; ++it) EXPECT_EQ(v[i++], TestFixture::value_to_key(*it));
}

TYPED_TEST(assoc_common, prefixed_with_includes_prefix) {
    TypeParam t = TestFixture::make_container("foo", "bar", "aa", "aaa", "aab", "aac", "aad", "ab");

    auto [first, last] = t.prefixed_with("aa");

    EXPECT_EQ(5u, std::distance(first, last));
}

TYPED_TEST(assoc_common, prefixed_with_empty_range) {
    TypeParam t = TestFixture::roman_trie;

    auto [first, last] = t.prefixed_with("rob");
    EXPECT_EQ(first, last);
}

TYPED_TEST(assoc_common, longest_match) {
    TypeParam t = TestFixture::make_container("foo", "foobar", "baz");
    auto it = t.longest_match("fooba");

    ASSERT_NE(t.end(), it);
    EXPECT_EQ("foo", TestFixture::value_to_key(*it));
}

TYPED_TEST(assoc_common, longest_match_has_key) {
    TypeParam t = TestFixture::make_container("foo", "foobar", "baz");
    auto it = t.longest_match("foobar");

    ASSERT_NE(t.end(), it);
    EXPECT_EQ("foobar", TestFixture::value_to_key(*it));
}

TYPED_TEST(assoc_common, longest_match_no_key) {
    TypeParam t = TestFixture::make_container("foo", "foobar", "baz");
    auto it = t.longest_match("qux");

    EXPECT_EQ(t.end(), it);
}

TYPED_TEST(assoc_common, longest_match_empty) {
    TypeParam t;
    auto it = t.longest_match("foo");

    EXPECT_EQ(t.end(), it);
}

TYPED_TEST(assoc_common, equals) {
    TypeParam t = TestFixture::roman_trie;
    TypeParam s = TestFixture::roman_trie;

    EXPECT_EQ(t, s);
}

TYPED_TEST(assoc_common, not_equals) {
    TypeParam t = TestFixture::roman_trie;
    TypeParam s = TestFixture::roman_trie;
    s.erase("romane");

    EXPECT_NE(t, s);
}

TYPED_TEST(assoc_common, greater) {
    TypeParam l = TestFixture::less_trie;
    TypeParam g = TestFixture::greater_trie;

    EXPECT_GT(g, l);
}

TYPED_TEST(assoc_common, greater_equals) {
    TypeParam l = TestFixture::less_trie;
    TypeParam g = TestFixture::greater_trie;

    EXPECT_GE(g, g);
    EXPECT_GE(g, l);
    EXPECT_GE(l, l);
}

TYPED_TEST(assoc_common, less) {
    TypeParam l = TestFixture::less_trie;
    TypeParam g = TestFixture::greater_trie;

    EXPECT_LT(l, g);
}

TYPED_TEST(assoc_common, less_equals) {
    TypeParam l = TestFixture::less_trie;
    TypeParam g = TestFixture::greater_trie;

    EXPECT_LE(l, l);
    EXPECT_LE(l, g);
    EXPECT_LE(g, g);
}

TYPED_TEST(assoc_common, typedefs) {
    using value_type = typename TypeParam::value_type;

    EXPECT_TRUE((std::is_same_v<std::string, typename TypeParam::key_type>));
    EXPECT_TRUE((std::is_same_v<value_type, typename TypeParam::value_type>));
    EXPECT_TRUE((std::is_same_v<std::size_t, typename TypeParam::size_type>));
    EXPECT_TRUE((std::is_same_v<std::ptrdiff_t, typename TypeParam::difference_type>));
    EXPECT_TRUE((std::is_same_v<rmr::identity<std::size_t>, typename TypeParam::key_mapper>));
    EXPECT_TRUE((std::is_same_v<std::allocator<value_type>, typename TypeParam::allocator_type>));
    EXPECT_TRUE((std::is_same_v<value_type&, typename TypeParam::reference>));
    EXPECT_TRUE((std::is_same_v<const value_type&, typename TypeParam::const_reference>));
    EXPECT_TRUE((std::is_same_v<value_type*, typename TypeParam::pointer>));
    EXPECT_TRUE((std::is_same_v<const value_type*, typename TypeParam::const_pointer>));

    EXPECT_TRUE(test::has_iterator<TypeParam>::value);
    EXPECT_TRUE(test::has_const_iterator<TypeParam>::value);
    EXPECT_TRUE(test::has_reverse_iterator<TypeParam>::value);
    EXPECT_TRUE(test::has_const_reverse_iterator<TypeParam>::value);
    EXPECT_TRUE(test::has_node_type<TypeParam>::value);
    EXPECT_TRUE(test::has_insert_return_type<TypeParam>::value);
}

// TODO inserts
// TODO emplaces
// TODO erase, count, find, extract, merge
// TODO swap
