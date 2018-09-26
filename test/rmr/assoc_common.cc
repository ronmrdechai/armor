// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#include <gtest/gtest.h>

#include "assoc.h"

template <typename T>
struct assoc_common : test::assoc_test<T>, testing::Test {};
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

TYPED_TEST(assoc_common, swap) {
    TypeParam t = TestFixture::make_container("foo");
    TypeParam s = TestFixture::make_container("bar");

    swap(t, s);

    EXPECT_EQ(t.end(), t.find("foo"));
    EXPECT_NE(t.end(), t.find("bar"));

    EXPECT_EQ(s.end(), s.find("bar"));
    EXPECT_NE(s.end(), s.find("foo"));
}

TYPED_TEST(assoc_common, swap_and_modify) {
    TypeParam t = TestFixture::make_container("foo");
    TypeParam s = TestFixture::make_container("bar");

    swap(t, s);
    t.erase("bar");

    EXPECT_EQ(t.end(), t.find("foo"));
    EXPECT_EQ(t.end(), t.find("bar"));

    EXPECT_EQ(s.end(), s.find("bar"));
    EXPECT_NE(s.end(), s.find("foo"));
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
    auto [it, inserted] = t.insert(v);

    EXPECT_TRUE(inserted);
    EXPECT_EQ("foo", TestFixture::value_to_key(*it));
}

TYPED_TEST(assoc_common, insert_moved) {
    TypeParam t;

    typename TypeParam::value_type v = TestFixture::key_to_value("foo");
    auto [it, inserted] = t.insert(std::move(v));

    EXPECT_TRUE(inserted);
    EXPECT_EQ("foo", TestFixture::value_to_key(*it));
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

TYPED_TEST(assoc_common, insert_prefix) {
    TypeParam t = TestFixture::make_container("foobar");
    auto [it, inserted] = t.insert( TestFixture::key_to_value("foo") );

    EXPECT_TRUE(inserted);
    EXPECT_EQ("foo", TestFixture::value_to_key(*it));
    EXPECT_EQ("foobar", TestFixture::value_to_key(*t.find("foobar")));
}

TYPED_TEST(assoc_common, insert_sufix) {
    TypeParam t = TestFixture::make_container("foo");
    auto [it, inserted] = t.insert( TestFixture::key_to_value("foobar") );

    EXPECT_TRUE(inserted);
    EXPECT_EQ("foobar", TestFixture::value_to_key(*it));
    EXPECT_EQ("foo", TestFixture::value_to_key(*t.find("foo")));
}

TYPED_TEST(assoc_common, insert_hint_size_change) {
    TypeParam t = TestFixture::make_container("foo");
    auto hint = t.find("foo");

    t.insert(hint, TestFixture::key_to_value("foobar"));
    EXPECT_EQ(2u, t.size());
}

TYPED_TEST(assoc_common, insert_hint_exists_size_no_change) {
    TypeParam t = TestFixture::make_container("foo", "foobar");
    auto hint = t.find("foo");

    t.insert(hint, TestFixture::key_to_value("foobar"));
    EXPECT_EQ(2u, t.size());
}

TYPED_TEST(assoc_common, insert_hint_return_value) {
    TypeParam t = TestFixture::make_container("foo");
    auto hint = t.find("foo");

    auto it = t.insert(hint, TestFixture::key_to_value("foobar"));
    EXPECT_EQ("foobar", TestFixture::value_to_key(*it));
}

TYPED_TEST(assoc_common, insert_hint_wrong_hint) {
    TypeParam t = TestFixture::make_container("bar");
    auto hint = t.find("bar");

    auto it = t.insert(hint, TestFixture::key_to_value("foobar"));
    EXPECT_NE(t.end(), t.find("barbar"));
    EXPECT_EQ("foobar", TestFixture::value_to_key(*it));
}

TYPED_TEST(assoc_common, insert_handle_size_change) {
    TypeParam t = TestFixture::roman_trie;
    typename TypeParam::node_type handle = t.extract("romulus");

    t.insert(std::move(handle));
    EXPECT_EQ(7u, t.size());
}

TYPED_TEST(assoc_common, insert_handle_exists_size_no_change) {
    TypeParam t = TestFixture::roman_trie;
    typename TypeParam::node_type handle = t.extract("romulus");

    TypeParam s = TestFixture::roman_trie;

    s.insert(std::move(handle));
    EXPECT_EQ(7u, s.size());
}

TYPED_TEST(assoc_common, insert_empty_handle_size_no_chage) {
    TypeParam t = TestFixture::roman_trie;
    typename TypeParam::node_type handle;

    t.insert(std::move(handle));
    EXPECT_EQ(7u, t.size());
}

TYPED_TEST(assoc_common, insert_handle_return_value) {
    TypeParam t = TestFixture::roman_trie;
    typename TypeParam::node_type handle = t.extract("romulus");

    auto insert_return = t.insert(std::move(handle));
    EXPECT_TRUE(insert_return.inserted);
    EXPECT_TRUE(insert_return.node.empty());
    EXPECT_EQ("romulus", TestFixture::value_to_key(*insert_return.position));
}

TYPED_TEST(assoc_common, insert_handle_exists_return_value) {
    TypeParam t = TestFixture::roman_trie;
    typename TypeParam::node_type handle = t.extract("romulus");

    TypeParam s = TestFixture::roman_trie;

    auto insert_return = s.insert(std::move(handle));
    EXPECT_FALSE(insert_return.inserted);
    EXPECT_FALSE(insert_return.node.empty());
    EXPECT_EQ("romulus", TestFixture::value_to_key(*insert_return.position));
}

TYPED_TEST(assoc_common, insert_empty_handle_return_value) {
    TypeParam t = TestFixture::roman_trie;
    typename TypeParam::node_type handle;

    auto insert_return = t.insert(std::move(handle));
    EXPECT_FALSE(insert_return.inserted);
    EXPECT_TRUE(insert_return.node.empty());
    EXPECT_EQ(t.end(), insert_return.position);
}

TYPED_TEST(assoc_common, insert_hint_handle_size_change) {
    TypeParam t = TestFixture::roman_trie;
    auto hint = t.find("romulus");

    t.insert(hint, t.extract("romulus"));
    EXPECT_EQ(7u, t.size());
}

TYPED_TEST(assoc_common, insert_hint_handle_exists_size_no_change) {
    TypeParam t = TestFixture::roman_trie;
    TypeParam s = TestFixture::roman_trie;
    auto hint = t.find("romulus");

    t.insert(hint, s.extract("romulus"));
    EXPECT_EQ(7u, t.size());
}

TYPED_TEST(assoc_common, insert_hint_handle_return_type) {
    TypeParam t = TestFixture::roman_trie;
    auto hint = t.find("romulus");

    auto it = t.insert(hint, t.extract("romulus"));
    EXPECT_EQ("romulus", TestFixture::value_to_key(*it));
}

TYPED_TEST(assoc_common, emplace_size_change) {
    TypeParam t;
    t.emplace( TestFixture::key_to_value("foo") );
    EXPECT_EQ(1u, t.size());
    t.emplace( TestFixture::key_to_value("bar") );
    EXPECT_EQ(2u, t.size());
}

TYPED_TEST(assoc_common, emplace_existing_size_no_change) {
    TypeParam t;
    t.emplace( TestFixture::key_to_value("foo") );
    EXPECT_EQ(1u, t.size());
    t.emplace( TestFixture::key_to_value("foo") );
    EXPECT_EQ(1u, t.size());
}

TYPED_TEST(assoc_common, emplace_return_value) {
    TypeParam t;
    auto [it, inserted] = t.emplace( TestFixture::key_to_value("foo") );

    EXPECT_TRUE(inserted);
    ASSERT_NE(t.end(), it);
    EXPECT_EQ("foo", TestFixture::value_to_key(*it));
}

TYPED_TEST(assoc_common, emplace_existing_return_value) {
    TypeParam t;

    t.emplace( TestFixture::key_to_value("foo") );

    auto [it, inserted] = t.emplace( TestFixture::key_to_value("foo") );

    EXPECT_FALSE(inserted);
    ASSERT_NE(t.end(), it);
    EXPECT_EQ("foo", TestFixture::value_to_key(*it));
}

TYPED_TEST(assoc_common, emplace_hint_size_change) {
    TypeParam t{ TestFixture::key_to_value("foo") };
    auto hint = t.find("foo");

    t.emplace_hint(hint, TestFixture::key_to_value("foobar"));
    EXPECT_EQ(2u, t.size());
}

TYPED_TEST(assoc_common, emplace_hint_existing_size_no_change) {
    TypeParam t{ TestFixture::key_to_value("foo"), TestFixture::key_to_value("foobar") };
    auto hint = t.find("foo");

    t.emplace_hint(hint, TestFixture::key_to_value("foobar"));
    EXPECT_EQ(2u, t.size());
}

TYPED_TEST(assoc_common, emplace_hint_return_value) {
    TypeParam t{ TestFixture::key_to_value("bar") };
    auto hint = t.find("bar");

    auto it = t.emplace_hint(hint, TestFixture::key_to_value("foobar"));
    EXPECT_EQ("foobar", TestFixture::value_to_key(*it));
}

TYPED_TEST(assoc_common, emplace_hint_wrong_hint) {
    TypeParam t{ TestFixture::key_to_value("bar") };
    auto hint = t.find("bar");

    auto it = t.emplace_hint(hint, TestFixture::key_to_value("foobar"));
    EXPECT_NE(t.end(), t.find("barbar"));
    EXPECT_EQ("foobar", TestFixture::value_to_key(*it));
}

TYPED_TEST(assoc_common, erase_not_empty) {
    TypeParam t = TestFixture::make_container("foo", "bar", "baz");

    t.erase("foo");
    EXPECT_EQ(t.end(), t.find("foo"));
    EXPECT_NE(t.end(), t.find("bar"));
    EXPECT_NE(t.end(), t.find("baz"));

    t.erase("bar");
    EXPECT_EQ(t.end(), t.find("bar"));
    EXPECT_NE(t.end(), t.find("baz"));
}

TYPED_TEST(assoc_common, erase_empty) {
    TypeParam t = TestFixture::make_container("foo");

    t.erase("foo");

    EXPECT_EQ(t.end(), t.find("foo"));
    // make sure container still works
    t.insert( TestFixture::key_to_value("bar") );
    EXPECT_NE(t.end(), t.find("bar"));
}

TYPED_TEST(assoc_common, erase_prefix) {
    TypeParam t = TestFixture::make_container("foo", "foobar", "bar");
    t.erase("foo");

    EXPECT_EQ(t.end(), t.find("foo"));
    EXPECT_NE(t.end(), t.find("foobar"));
    EXPECT_NE(t.end(), t.find("bar"));
}

TYPED_TEST(assoc_common, erase_suffix) {
    TypeParam t = TestFixture::make_container("foo", "foobar", "bar");
    t.erase("foobar");

    EXPECT_EQ(t.end(), t.find("foobar"));
    EXPECT_NE(t.end(), t.find("foo"));
    EXPECT_NE(t.end(), t.find("bar"));
}

TYPED_TEST(assoc_common, erase_iterator) {
    TypeParam t = TestFixture::make_container("foo", "bar", "baz");

    /* typename TypeParam::const_iterator it = t.find("foo"); */
    auto it = t.find("foo");
    t.erase(it);

    EXPECT_EQ(t.end(), t.find("foo"));
    EXPECT_NE(t.end(), t.find("bar"));
    EXPECT_NE(t.end(), t.find("baz"));
}

TYPED_TEST(assoc_common, erase_size_change) {
    TypeParam t = TestFixture::make_container("foo", "bar", "baz");
    t.erase("foo");
    EXPECT_EQ(2u, t.size());

    t.erase("bar");
    EXPECT_EQ(1u, t.size());
}

TYPED_TEST(assoc_common, erase_not_existsing_size_no_change) {
    TypeParam t = TestFixture::make_container("foo", "bar", "baz");
    t.erase("foobar");
    EXPECT_EQ(3u, t.size());
}

TYPED_TEST(assoc_common, erase_not_existsing) {
    TypeParam t = TestFixture::make_container("bar", "baz");

    t.erase("foo");
    EXPECT_NE(t.end(), t.find("bar"));
    EXPECT_NE(t.end(), t.find("baz"));
}

TYPED_TEST(assoc_common, erase_return_value) {
    TypeParam t = TestFixture::make_container("foo", "bar", "baz");

    EXPECT_EQ(1u, t.erase("foo"));
    EXPECT_EQ(0u, t.erase("foo"));
}

TYPED_TEST(assoc_common, erase_range) {
    TypeParam t = TestFixture::make_container("foo", "bar", "bax", "bay", "baz");

    auto [first, last] = t.prefixed_with("ba");
    t.erase(first, last);

    EXPECT_NE(t.end(), t.find("foo"));
    EXPECT_NE(t.end(), t.find("baz"));

    EXPECT_EQ(t.end(), t.find("bar"));
    EXPECT_EQ(t.end(), t.find("bax"));
    EXPECT_EQ(t.end(), t.find("bay"));
}

TYPED_TEST(assoc_common, erase_range_return_value) {
    TypeParam t = TestFixture::make_container("foo", "bar", "bax", "bay", "baz");

    auto [first, last] = t.prefixed_with("ba");
    EXPECT_EQ(last, t.erase(first, last));
}

TYPED_TEST(assoc_common, erase_empty_range) {
    TypeParam t = TestFixture::make_container("foo", "bar", "baz");

    auto first = t.find("foo");
    auto last  = t.find("foo");

    t.erase(first, last);
    EXPECT_NE(t.end(), t.find("foo"));
    EXPECT_NE(t.end(), t.find("bar"));
    EXPECT_NE(t.end(), t.find("baz"));
}

TYPED_TEST(assoc_common, default_count_is_zero) {
    TypeParam t;
    EXPECT_EQ(0u, t.count("foo"));
}

TYPED_TEST(assoc_common, count_increase_after_insert) {
    TypeParam t;
    t.insert( TestFixture::key_to_value("foo") );
    EXPECT_EQ(1u, t.count("foo"));
}

TYPED_TEST(assoc_common, find_existant) {
    TypeParam t;
    t.insert( TestFixture::key_to_value("foo") );

    auto it = t.find("foo");
    ASSERT_NE(t.end(), t.find("foo"));
    EXPECT_EQ("foo", TestFixture::value_to_key(*it));
}

TYPED_TEST(assoc_common, find_non_existant) {
    TypeParam t;

    EXPECT_EQ(t.end(), t.find("foo"));
}

TYPED_TEST(assoc_common, extract_erases) {
    TypeParam t = TestFixture::make_container("foo", "bar", "baz");

    t.extract("foo");
    EXPECT_EQ(t.end(), t.find("foo"));
}

TYPED_TEST(assoc_common, extract_size_change) {
    TypeParam t = TestFixture::make_container("foo", "bar", "baz");

    t.extract("foo");
    EXPECT_EQ(2u, t.size());
}

TYPED_TEST(assoc_common, extract_gives_valid_handle) {
    TypeParam t = TestFixture::make_container("foo", "bar", "baz");
    auto nh = t.extract("foo");

    EXPECT_FALSE(nh.empty());
}

TYPED_TEST(assoc_common, merge_all) {
    TypeParam t = TestFixture::make_container("foo", "bar", "baz");
    TypeParam s = TestFixture::make_container("qux", "quux");

    t.merge(s);

    EXPECT_EQ(5u, t.size());
    EXPECT_EQ(0u, s.size());
    EXPECT_NE(t.end(), t.find("qux"));
    EXPECT_NE(t.end(), t.find("quux"));
    EXPECT_EQ(s.end(), s.find("qux"));
    EXPECT_EQ(s.end(), s.find("quux"));
}

TYPED_TEST(assoc_common, merge_partial) {
    TypeParam t = TestFixture::make_container("foo", "bar", "baz");
    TypeParam s = TestFixture::make_container("baz", "qux");

    t.merge(s);

    EXPECT_EQ(4u, t.size());
    EXPECT_EQ(1u, s.size());
    EXPECT_NE(s.end(), s.find("baz"));
}

TYPED_TEST(assoc_common, merge_moved) {
    TypeParam t = TestFixture::make_container("foo", "bar", "baz");
    TypeParam s = TestFixture::make_container("qux", "quux");

    t.merge(std::move(s));

    EXPECT_EQ(5u, t.size());
    EXPECT_EQ(0u, s.size());
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

TYPED_TEST(assoc_common, prefixed_whole_container) {
    TypeParam t = TestFixture::make_container("foo", "bar", "aa", "aaa", "aab", "aac", "aad", "ab");

    auto [first, last] = t.prefixed_with("");

    EXPECT_EQ(t.begin(), first);
    EXPECT_EQ(t.end(), last);
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

TYPED_TEST(assoc_common, node_handle_extracted_is_not_empty_or_false) {
    TypeParam t = TestFixture::roman_trie;
    auto&& nh = t.extract("romulus");
    EXPECT_FALSE(nh.empty());
    EXPECT_TRUE(bool(nh));
}

TYPED_TEST(assoc_common, node_handle_default_is_empty_and_false) {
    typename TypeParam::node_type nh;
    EXPECT_TRUE(nh.empty());
    EXPECT_FALSE(bool(nh));
}

TYPED_TEST(assoc_common, node_handle_allocator_equals_container_allocator) {
    TypeParam t = TestFixture::roman_trie;
    auto&& nh = t.extract("romulus");
    EXPECT_EQ(nh.get_allocator(), t.get_allocator());
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

    EXPECT_TRUE(TestFixture::has_iterator);
    EXPECT_TRUE(TestFixture::has_const_iterator);
    EXPECT_TRUE(TestFixture::has_reverse_iterator);
    EXPECT_TRUE(TestFixture::has_const_reverse_iterator);
    EXPECT_TRUE(TestFixture::has_node_type);
    EXPECT_TRUE(TestFixture::has_insert_return_type);
}

TYPED_TEST(assoc_common, DISABLED_does_not_leak) {
    FAIL() << "Not implemented";
}
