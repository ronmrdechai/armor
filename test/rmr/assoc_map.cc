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

TYPED_TEST(assoc_map, access_read) {} // TODO
TYPED_TEST(assoc_map, access_write) {} // TODO

TYPED_TEST(assoc_map, safe_access_read) {} // TODO
TYPED_TEST(assoc_map, safe_access_write) {} // TODO
TYPED_TEST(assoc_map, safe_access_throws) {} // TODO

TYPED_TEST(assoc_map, insert_or_assign_and_access) {} // TODO
TYPED_TEST(assoc_map, insert_or_assign_twice) {} // TODO
TYPED_TEST(assoc_map, insert_or_assign_hint) {} // TODO
TYPED_TEST(assoc_map, insert_or_assign_hint_exists) {}  // TODO
TYPED_TEST(assoc_map, insert_or_assign_hint_wrong_hint) {} // TODO

TYPED_TEST(assoc_map, try_emplace_twice) {} // TODO
TYPED_TEST(assoc_map, try_emplace_hint) {} // TODO
TYPED_TEST(assoc_map, try_emplace_hint_exists) {} // TODO
TYPED_TEST(assoc_map, try_emplace_hint_wrong_hint) {} // TODO

TYPED_TEST(assoc_map, node_type_key_access) {} // TODO
TYPED_TEST(assoc_map, node_type_key_change) {} // TODO
TYPED_TEST(assoc_map, node_type_value_access) {} // TODO
TYPED_TEST(assoc_map, node_type_value_change) {} // TODO

TYPED_TEST(assoc_map, typedefs) {
    using key_type    = typename TypeParam::key_type;
    using mapped_type = typename TypeParam::mapped_type;
    EXPECT_TRUE(TestFixture::has_mapped_type);
    EXPECT_TRUE((std::is_same_v<typename TypeParam::value_type, std::pair<key_type, mapped_type>>));
}

TYPED_TEST(assoc_map, does_not_leak) {
    FAIL() << "Not implemented";
}
