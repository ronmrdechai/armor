// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#include <gtest/gtest.h>

#include "assoc.h"

template <typename T>
struct assoc_set : test::assoc_test<T>, testing::Test {};
TYPED_TEST_CASE(assoc_set, assoc_set_types);

TYPED_TEST(assoc_set, node_handle_value_access) {
    TypeParam t = TestFixture::roman_trie;
    auto&& nh = t.extract("romulus");
    EXPECT_EQ("romulus", nh.value());
}

TYPED_TEST(assoc_set, node_handle_value_change) {
    TypeParam t = TestFixture::roman_trie;

    auto&& nh = t.extract("romulus");
    nh.value() = "rome";
    t.insert(std::move(nh));

    EXPECT_TRUE(t.count("rome"));
}

TYPED_TEST(assoc_set, typedefs) {
    EXPECT_FALSE(TestFixture::has_mapped_type);
    EXPECT_TRUE((std::is_same_v<typename TypeParam::value_type, typename TypeParam::key_type>));
}
