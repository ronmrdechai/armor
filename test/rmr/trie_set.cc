// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)
//
#include <gtest/gtest.h>

#include <rmr/trie_set.h>

using trie_set = rmr::trie_set<127>;

namespace fixtures {

const trie_set roman_trie{
    "romane", "romanus", "romulus", "rubens", "ruber", "rubicon", "rubicundus"
};

} // namespace fixtures

TEST(trie_set, roman_trie_size) {
    trie_set t = fixtures::roman_trie;
    EXPECT_EQ(7u, t.size());
    EXPECT_EQ(7u, std::distance(t.begin(), t.end()));
}
