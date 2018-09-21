// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#include <gtest/gtest.h>

#include "assoc.h"

template <typename> struct assoc_common : testing::Test {};
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
    TypeParam t = test::roman_trie<TypeParam>;
    EXPECT_EQ(7u, t.size());
}

TYPED_TEST(assoc_common, roman_trie_distance_is_7) {
    TypeParam t = test::roman_trie<TypeParam>;
    EXPECT_EQ(7u, std::distance(t.begin(), t.end()));
}

TYPED_TEST(assoc_common, empty_after_clear) {
    TypeParam t = test::roman_trie<TypeParam>;
    t.clear();
    test::assert_empty(t);
}

TYPED_TEST(assoc_common, empty_after_move_construct) {
    TypeParam t = test::roman_trie<TypeParam>;
    TypeParam(std::move(t));

    test::assert_empty(t);
}

TYPED_TEST(assoc_common, empty_after_move_assign) {
    TypeParam t = test::roman_trie<TypeParam>;
    TypeParam{} = std::move(t);

    test::assert_empty(t);
}

TYPED_TEST(assoc_common, iteration_is_sorted) {
    TypeParam t = test::roman_trie<TypeParam>;

    std::vector<typename TypeParam::key_type> unsorted;
    std::transform(t.begin(), t.end(), std::back_inserter(unsorted), test::value_to_key<TypeParam>);

    std::vector<typename TypeParam::key_type> sorted = unsorted;
    std::sort(sorted.begin(), sorted.end());

    EXPECT_EQ(unsorted, sorted);
}

TYPED_TEST(assoc_common, reverse_iteration_is_reverse_sorted) {
    TypeParam t = test::roman_trie<TypeParam>;

    std::vector<typename TypeParam::key_type> unsorted;
    std::transform(t.rbegin(), t.rend(), std::back_inserter(unsorted), test::value_to_key<TypeParam>);

    std::vector<typename TypeParam::key_type> sorted = unsorted;
    std::sort(sorted.begin(), sorted.end());
    std::reverse(sorted.begin(), sorted.end());

    EXPECT_EQ(unsorted, sorted);
}

TYPED_TEST(assoc_common, reverse_iteration_covers_whole_container) {
    TypeParam t = test::roman_trie<TypeParam>;
    EXPECT_EQ(std::distance(t.begin(), t.end()), std::distance(t.rbegin(), t.rend()));
}

TYPED_TEST(assoc_common, reverse_iteration_is_reversed) {
    TypeParam t = test::roman_trie<TypeParam>;

    std::vector<typename TypeParam::key_type> not_reversed;
    std::transform(t.begin(), t.end(), std::back_inserter(not_reversed), test::value_to_key<TypeParam>);

    std::vector<typename TypeParam::key_type> reversed;
    std::transform(t.rbegin(), t.rend(), std::back_inserter(reversed), test::value_to_key<TypeParam>);

    std::reverse(not_reversed.begin(), not_reversed.end());

    EXPECT_EQ(not_reversed, reversed);
}

TYPED_TEST(assoc_common, max_size_is_uint64_max) {
    TypeParam t;
    EXPECT_EQ(UINT64_MAX, t.max_size());
}

TYPED_TEST(assoc_common, insert_size_change) {
    TypeParam t;
    t.insert( test::key_to_value<TypeParam>("foo") );
    EXPECT_EQ(1u, t.size());
    t.insert( test::key_to_value<TypeParam>("bar") );
    EXPECT_EQ(2u, t.size());
}

TYPED_TEST(assoc_common, insert_existing_size_no_change) {
    TypeParam t;
    t.insert( test::key_to_value<TypeParam>("foo") );
    EXPECT_EQ(1u, t.size());
    t.insert( test::key_to_value<TypeParam>("foo") );
    EXPECT_EQ(1u, t.size());
}

TYPED_TEST(assoc_common, insert_return_value) {
    TypeParam t;
    auto [it, inserted] = t.insert( test::key_to_value<TypeParam>("foo") );

    EXPECT_TRUE(inserted);
    ASSERT_NE(t.end(), it);
    EXPECT_EQ("foo", test::value_to_key<TypeParam>(*it));
}

TYPED_TEST(assoc_common, insert_existing_return_value) {
    TypeParam t;

    t.insert( test::key_to_value<TypeParam>("foo") );

    auto [it, inserted] = t.insert( test::key_to_value<TypeParam>("foo") );

    EXPECT_FALSE(inserted);
    ASSERT_NE(t.end(), it);
    EXPECT_EQ("foo", test::value_to_key<TypeParam>(*it));
}

TYPED_TEST(assoc_common, insert_lvalue_reference) {
    TypeParam t;

    typename TypeParam::value_type v = test::key_to_value<TypeParam>("foo");
    t.insert(v);
    EXPECT_EQ(1u, t.size());
}

TYPED_TEST(assoc_common, insert_range_size_increase) {
    TypeParam t;

    std::vector<typename TypeParam::value_type> v{
        test::key_to_value<TypeParam>("foo"),
        test::key_to_value<TypeParam>("bar"),
        test::key_to_value<TypeParam>("baz")
    };
    t.insert(v.begin(), v.end());

    EXPECT_EQ(3u, t.size());
}

TYPED_TEST(assoc_common, insert_initializer_list_size_increase) {
    TypeParam t;

    t.insert({
        test::key_to_value<TypeParam>("foo"),
        test::key_to_value<TypeParam>("bar"),
        test::key_to_value<TypeParam>("baz")
    });

    EXPECT_EQ(3u, t.size());
}

TYPED_TEST(assoc_common, equals) {
    TypeParam t = test::roman_trie<TypeParam>;
    TypeParam s = test::roman_trie<TypeParam>;

    EXPECT_EQ(t, s);
}

TYPED_TEST(assoc_common, not_equals) {
    TypeParam t = test::roman_trie<TypeParam>;
    TypeParam s = test::roman_trie<TypeParam>;
    s.erase("romane");

    EXPECT_NE(t, s);
}

TYPED_TEST(assoc_common, greater) {
    TypeParam l = test::less_trie<TypeParam>;
    TypeParam g = test::greater_trie<TypeParam>;

    EXPECT_GT(g, l);
}

TYPED_TEST(assoc_common, greater_equals) {
    TypeParam l = test::less_trie<TypeParam>;
    TypeParam g = test::greater_trie<TypeParam>;

    EXPECT_GE(g, g);
    EXPECT_GE(g, l);
    EXPECT_GE(l, l);
}

TYPED_TEST(assoc_common, less) {
    TypeParam l = test::less_trie<TypeParam>;
    TypeParam g = test::greater_trie<TypeParam>;

    EXPECT_LT(l, g);
}

TYPED_TEST(assoc_common, less_equals) {
    TypeParam l = test::less_trie<TypeParam>;
    TypeParam g = test::greater_trie<TypeParam>;

    EXPECT_LE(l, l);
    EXPECT_LE(l, g);
    EXPECT_LE(g, g);
}

// TODO constructors
// TODO assignment
// TODO inserts
// TODO emplaces
// TODO erase, count, find, extract, merge
// TODO swap
// TODO prefixed_with, longest_match
// TODO typedefs
