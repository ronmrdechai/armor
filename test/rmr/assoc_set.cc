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

TYPED_TEST(assoc_set, node_handle_swap) {} // TODO
TYPED_TEST(assoc_set, node_handle_value_access) {} // TODO
TYPED_TEST(assoc_set, node_handle_value_change) {} // TODO

TYPED_TEST(assoc_set, typedefs) {
    EXPECT_FALSE(TestFixture::has_mapped_type);
    EXPECT_TRUE((std::is_same_v<typename TypeParam::value_type, typename TypeParam::key_type>));
}
