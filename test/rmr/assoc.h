// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <rmr/trie_map.h>
#include <rmr/trie_set.h>

// using structs and not typedefs for CTest pretty types.
struct trie_map : rmr::trie_map<int, 127> { using rmr::trie_map<int, 127>::trie_map; };
struct trie_set : rmr::trie_set<127>      { using rmr::trie_set<127>::trie_set;      };

using assoc_map_types    = testing::Types<trie_map>;
using assoc_set_types    = testing::Types<trie_set>;
using assoc_common_types = testing::Types<trie_map, trie_set>;

namespace test {

template <typename T> T roman_trie;
template <> trie_map roman_trie<trie_map>{
    {"romane", 1},
    {"romanus", 1},
    {"romulus", 1},
    {"rubens", 1},
    {"ruber", 1},
    {"rubicon", 1},
    {"rubicundus", 1}
};
template <> trie_set roman_trie<trie_set>{
    "romane", "romanus", "romulus", "rubens", "ruber", "rubicon", "rubicundus"
};

template <typename T> T less_trie;
template <> trie_map less_trie<trie_map>{ {"aaa", 1}, {"bbb", 1}, {"ccc", 1} };
template <> trie_set less_trie<trie_set>{ "aaa", "bbb", "ccc" };

template <typename T> T greater_trie;
template <> trie_map greater_trie<trie_map>{ {"bbb", 1}, {"ccc", 1}, {"ddd", 1} };
template <> trie_set greater_trie<trie_set>{ "bbb", "ccc", "ddd" };

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
