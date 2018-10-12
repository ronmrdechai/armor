// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#include <gtest/gtest.h>

#include "assoc.h"

template <typename T>
struct assoc_map : test::assoc_test<T>, testing::Test {};
TYPED_TEST_CASE(assoc_map, assoc_map_types);

TYPED_TEST(assoc_map, access_read) {
    TypeParam t{ {"foo", 1}, {"foobar", 2}, {"baz", 3} };
    EXPECT_EQ(1, t["foo"]);
    EXPECT_EQ(2, t["foobar"]);
    EXPECT_EQ(3, t["baz"]);
}

TYPED_TEST(assoc_map, access_write) {
    TypeParam t{ {"foo", 1}, {"foobar", 2}, {"baz", 3} };

    t["foo"]    = 42;
    t["foobar"] = 43;
    t["baz"]    = 44;
    t["qux"]    = 45;

    EXPECT_EQ(42, t["foo"]);
    EXPECT_EQ(43, t["foobar"]);
    EXPECT_EQ(44, t["baz"]);
    EXPECT_EQ(45, t["qux"]);
}

TYPED_TEST(assoc_map, safe_access_read) {
    TypeParam t{ {"foo", 1}, {"foobar", 2}, {"baz", 3} };
    EXPECT_EQ(1, t.at("foo"));
    EXPECT_EQ(2, t.at("foobar"));
    EXPECT_EQ(3, t.at("baz"));
}

TYPED_TEST(assoc_map, safe_access_write) {
    TypeParam t{ {"foo", 1}, {"foobar", 2}, {"baz", 3} };

    t.at("foo")    = 42;
    t.at("foobar") = 43;
    t.at("baz")    = 44;

    EXPECT_EQ(42, t.at("foo"));
    EXPECT_EQ(43, t.at("foobar"));
    EXPECT_EQ(44, t.at("baz"));
}

TYPED_TEST(assoc_map, safe_access_throws) {
    TypeParam t;

    ASSERT_THROW(t.at("foo"), std::out_of_range);
    ASSERT_THROW(t.at("foo") = 1, std::out_of_range);
}

TYPED_TEST(assoc_map, insert_or_assign_return_value) {
    TypeParam t;
    auto [it, inserted] = t.insert_or_assign("foo", 1);

    EXPECT_TRUE(inserted);
    EXPECT_EQ(1, it->second);
    EXPECT_NE(t.end(), t.find("foo"));
}

TYPED_TEST(assoc_map, insert_or_assign_existing_return_value) {
    TypeParam t{ {"foo", 42} };
    auto [it, inserted] = t.insert_or_assign("foo", 1);

    EXPECT_FALSE(inserted);
    EXPECT_EQ(1, it->second);
    EXPECT_NE(t.end(), t.find("foo"));
}

TYPED_TEST(assoc_map, insert_or_assign_hint_return_value) {
    TypeParam t{ {"foo", 0} };
    auto it = t.insert_or_assign(t.find("foo"), "foobar", 1);

    EXPECT_EQ(1, it->second);
    EXPECT_NE(t.end(), t.find("foobar"));
}

TYPED_TEST(assoc_map, insert_or_assign_hint_existing_return_value) {
    TypeParam t{ {"foo", 0}, {"foobar", 42} };
    auto it = t.insert_or_assign(t.find("foo"), "foobar", 1);

    EXPECT_EQ(1, it->second);
    EXPECT_NE(t.end(), t.find("foobar"));
}

TYPED_TEST(assoc_map, try_emplace_return_value) {
    TypeParam t;
    auto [it, inserted] = t.try_emplace("foo", 1);

    EXPECT_TRUE(inserted);
    EXPECT_EQ(1, it->second);
    EXPECT_NE(t.end(), t.find("foo"));
}

TYPED_TEST(assoc_map, try_emplace_existing_return_value) {
    TypeParam t{ {"foo", 42} };
    auto [it, inserted] = t.try_emplace("foo", 1);

    EXPECT_FALSE(inserted);
    EXPECT_EQ(42, it->second);
    EXPECT_NE(t.end(), t.find("foo"));
}

TYPED_TEST(assoc_map, try_emplace_hint_return_value) {
    TypeParam t{ {"foo", 0} };
    auto it = t.try_emplace(t.find("foo"), "foobar", 1);

    EXPECT_EQ(1, it->second);
    EXPECT_NE(t.end(), t.find("foobar"));
}

TYPED_TEST(assoc_map, try_emplace_hint_existing_return_value) {
    TypeParam t{ {"foo", 0}, {"foobar", 42} };
    auto it = t.try_emplace(t.find("foo"), "foobar", 1);

    EXPECT_EQ(42, it->second);
    EXPECT_NE(t.end(), t.find("foobar"));
}

TYPED_TEST(assoc_map, node_handle_key_access) {
    TypeParam t = TestFixture::roman_trie;
    auto&& nh = t.extract("romulus");
    EXPECT_EQ("romulus", nh.key());
}

TYPED_TEST(assoc_map, node_handle_key_change) {
    TypeParam t = TestFixture::roman_trie;
    t["romulus"] = 42;

    auto&& nh = t.extract("romulus");
    nh.key() = "rome";
    t.insert(std::move(nh));

    ASSERT_TRUE(t.count("rome"));
    EXPECT_EQ(42, t.at("rome"));
}

TYPED_TEST(assoc_map, node_handle_mapped_access) {
    TypeParam t = TestFixture::roman_trie;
    t["romulus"] = 42;

    auto&& nh = t.extract("romulus");
    EXPECT_EQ(42, nh.mapped());
}

TYPED_TEST(assoc_map, node_handle_mapped_change) {
    TypeParam t = TestFixture::roman_trie;

    auto&& nh = t.extract("romulus");
    nh.mapped() = 42;
    t.insert(std::move(nh));

    ASSERT_TRUE(t.count("romulus"));
    EXPECT_EQ(42, t["romulus"]);
}

TYPED_TEST(assoc_map, typedefs) {
    using key_type    = typename TypeParam::key_type;
    using mapped_type = typename TypeParam::mapped_type;
    EXPECT_TRUE(TestFixture::has_mapped_type);
    EXPECT_TRUE((std::is_same_v<typename TypeParam::value_type, std::pair<const key_type, mapped_type>>));
}
