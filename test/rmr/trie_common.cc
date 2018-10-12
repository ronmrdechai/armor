// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#include <gtest/gtest.h>

#include "assoc.h"

template <typename T>
struct trie_common : test::assoc_test<T>, testing::Test {};
TYPED_TEST_CASE(trie_common, trie_common_types);

TYPED_TEST(trie_common, insert_hint_wrong_hint) {
    TypeParam t = TestFixture::make_container("bar");
    auto hint = t.find("bar");

    auto it = t.insert(hint, TestFixture::key_to_value("foobar"));
    EXPECT_NE(t.end(), t.find("barbar"));
    EXPECT_EQ("foobar", TestFixture::value_to_key(*it));
}

TYPED_TEST(trie_common, emplace_hint_wrong_hint) {
    TypeParam t{ TestFixture::key_to_value("bar") };
    auto hint = t.find("bar");

    auto it = t.emplace_hint(hint, TestFixture::key_to_value("foobar"));
    EXPECT_NE(t.end(), t.find("barbar"));
    EXPECT_EQ("foobar", TestFixture::value_to_key(*it));
}

TYPED_TEST(trie_common, insert_empty_string) {
    TypeParam t;
    auto [it, inserted] = t.insert( TestFixture::key_to_value("") );

    EXPECT_TRUE(inserted);
    EXPECT_EQ("", TestFixture::value_to_key(*it));
    EXPECT_NE(t.end(), t.find(""));
}

TYPED_TEST(trie_common, key_map) {
    EXPECT_TRUE(TestFixture::has_key_mapper);

    TypeParam t;
    EXPECT_TRUE((std::is_same_v<decltype(t.key_map()), typename TypeParam::key_mapper>));
}
