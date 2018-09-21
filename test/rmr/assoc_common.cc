// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#include <gtest/gtest.h>

#include <rmr/trie_map.h>
#include <rmr/trie_set.h>

// using structs and not typedefs for CTest pretty types.
struct trie_map : rmr::trie_map<int, 127> { using rmr::trie_map<int, 127>::trie_map; };
struct trie_set : rmr::trie_set<127>      { using rmr::trie_set<127>::trie_set;      };

template <typename> struct assoc_common : public testing::Test {};
using assoc_common_types = testing::Types<trie_map, trie_set>;
TYPED_TEST_CASE(assoc_common, assoc_common_types);

namespace test {

template <typename T> T roman_trie() = delete;

template <> trie_map roman_trie<trie_map>() {
    return trie_map {
        {"romane", 1},
        {"romanus", 1},
        {"romulus", 1},
        {"rubens", 1},
        {"ruber", 1},
        {"rubicon", 1},
        {"rubicundus", 1}
    };
}

template <> trie_set roman_trie<trie_set>() {
    return trie_set {
        "romane", "romanus", "romulus", "rubens", "ruber", "rubicon", "rubicundus"
    };
}

template <typename T> typename T::value_type key_to_value(typename T::key_type) = delete;

template <> typename trie_map::value_type key_to_value<trie_map>(typename trie_map::key_type k) {
    return { k, typename trie_map::mapped_type{} };
}

template <> typename trie_set::value_type key_to_value<trie_set>(typename trie_set::key_type k) {
    return k;
}

template <typename T> typename T::key_type value_to_key(typename T::value_type) = delete;

template <> typename trie_map::key_type value_to_key<trie_map>(typename trie_map::value_type v) {
    return v.first;
}

template <> typename trie_set::key_type value_to_key<trie_set>(typename trie_set::value_type v) {
    return v;
}

template <typename T> void assert_empty(const T& t) {
    EXPECT_TRUE(t.empty());
    EXPECT_EQ(0u, t.size());
    EXPECT_EQ(0u, std::distance(t.begin(), t.end()));
}

} // namespace test

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
    TypeParam t = test::roman_trie<TypeParam>();
    EXPECT_EQ(7u, t.size());
}

TYPED_TEST(assoc_common, roman_trie_distance_is_7) {
    TypeParam t = test::roman_trie<TypeParam>();
    EXPECT_EQ(7u, std::distance(t.begin(), t.end()));
}

TYPED_TEST(assoc_common, empty_after_clear) {
    TypeParam t = test::roman_trie<TypeParam>();
    t.clear();
    test::assert_empty(t);
}

TYPED_TEST(assoc_common, empty_after_move_construct) {
    TypeParam t = test::roman_trie<TypeParam>();
    TypeParam(std::move(t));

    test::assert_empty(t);
}

TYPED_TEST(assoc_common, empty_after_move_assign) {
    TypeParam t = test::roman_trie<TypeParam>();
    TypeParam{} = std::move(t);

    test::assert_empty(t);
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

// TODO iteration, reverse iteration
// TODO constructors
// TODO assignment
// TODO inserts
// TODO emplaces
// TODO erase, count, find, extract, merge
// TODO swap
// TODO prefixed_with, longest_match
// TODO typedefs
