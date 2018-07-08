#include <gtest/gtest.h>

#include <moat/datastruct/radix_trie.h>

using radix_trie = moat::radix_trie<int, 127, moat::trie::identity>;

TEST(radix_trie, insertion_and_access) {
    radix_trie rt;
    rt["foo"] = 1;
    rt["bar"] = 2;
    EXPECT_EQ(1, rt["foo"]);
    EXPECT_EQ(2, rt["bar"]);
}

TEST(radix_trie, safe_access_read) {
}

TEST(radix_trie, safe_access_write) {
}

TEST(radix_trie, safe_access_throws) {
}

TEST(radix_trie, default_is_empty) {
}

TEST(radix_trie, not_empty_after_insert) {
}

TEST(radix_trie, empty_after_clear) {
}

TEST(radix_trie, default_size_is_zero) {
}

TEST(radix_trie, size_increase_after_write) {
}

TEST(radix_trie, default_count_is_zero) {
}

TEST(radix_trie, count_increase_after_write) {
}
