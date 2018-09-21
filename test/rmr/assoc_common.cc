// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#include <gtest/gtest.h>

#include <rmr/trie_map.h>
#include <rmr/trie_set.h>

using trie_map = rmr::trie_map<int, 127>;
using trie_set = rmr::trie_set<127>;

template <typename> struct assoc_common : public testing::Test {};
using assoc_common_types = testing::Types<trie_map, trie_set>;
TYPED_TEST_CASE(assoc_common, assoc_common_types);

template <typename> struct fixtures;
template <> struct fixtures<trie_map> {
    static trie_map roman_trie() {
        return trie_map{
            {"romane", 1},
            {"romanus", 1},
            {"romulus", 1},
            {"rubens", 1},
            {"ruber", 1},
            {"rubicon", 1},
            {"rubicundus", 1}
        };
    }
};
template <> struct fixtures<trie_set> {
    static trie_set roman_trie() {
        return trie_set{
            "romane", "romanus", "romulus", "rubens", "ruber", "rubicon", "rubicundus"
        };
    }
};

TYPED_TEST(assoc_common, default_is_empty) {
    TypeParam t;
    EXPECT_TRUE(t.empty());
}
