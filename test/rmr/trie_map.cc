// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#include <gtest/gtest.h>

#include "assoc.h"

TEST(trie_map, insert_or_assign_hint_wrong_hint) {
    trie_map t;
    auto [hint, _] = t.insert( {"bar", 0} );

    auto it = t.insert_or_assign(hint, "foobar", 1);

    auto barbar_it = t.find("barbar");

    EXPECT_EQ(it, barbar_it);
    EXPECT_EQ("foobar", it->first);
    EXPECT_EQ(1, it->second);

    auto foobar_it = t.find("foobar");
    EXPECT_EQ(t.end(), foobar_it);
}

TEST(trie_map, try_emplace_hint_wrong_hint) {
    trie_map t;
    auto [hint, _] = t.insert( {"bar", 0} );

    auto it = t.try_emplace(hint, "foobar", 1);

    auto barbar_it = t.find("barbar");

    EXPECT_EQ(it, barbar_it);
    EXPECT_EQ("foobar", it->first);
    EXPECT_EQ(1, it->second);

    auto foobar_it = t.find("foobar");
    EXPECT_EQ(t.end(), foobar_it);
}
