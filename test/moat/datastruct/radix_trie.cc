#include <gtest/gtest.h>

#include <moat/datastruct/radix_trie.h>

using radix_trie = moat::ascii_trie<int>;

TEST(radix_trie, insertion_and_access) {
    radix_trie rt;
    rt["foo"] = 1;
    rt["bar"] = 2;
    EXPECT_EQ(1, rt["foo"]);
    EXPECT_EQ(2, rt["bar"]);
}

TEST(radix_trie, safe_access_read) {
    radix_trie rt;
    rt["foo"] = 1;
    rt["bar"] = 2;
    EXPECT_EQ(1, rt.at("foo"));
    EXPECT_EQ(2, rt.at("bar"));
}

TEST(radix_trie, safe_access_write) {
    radix_trie rt;
    rt["foo"] = 1;
    rt.at("foo") = 2;
    EXPECT_EQ(2, rt["foo"]);
}

TEST(radix_trie, safe_access_throws) {
    radix_trie rt;

    ASSERT_THROW(rt.at("foo"), std::out_of_range);
    ASSERT_THROW(rt.at("foo") = 1, std::out_of_range);
}

TEST(radix_trie, default_is_empty) {
    radix_trie rt;
    EXPECT_TRUE(rt.empty());
}

TEST(radix_trie, not_empty_after_insert) {
    radix_trie rt;
    rt["foo"] = 1;
    EXPECT_FALSE(rt.empty());
}

TEST(radix_trie, empty_after_clear) {
    radix_trie rt;
    rt["foo"] = 1;
    rt.clear();
    EXPECT_TRUE(rt.empty());
}

TEST(radix_trie, default_size_is_zero) {
    radix_trie rt;
    EXPECT_EQ(0u, rt.size());
}

TEST(radix_trie, size_increase_after_write) {
    radix_trie rt;
    rt["foo"] = 1;
    EXPECT_EQ(1u, rt.size());
    rt["bar"] = 1;
    EXPECT_EQ(2u, rt.size());
}

TEST(radix_trie, default_count_is_zero) {
    radix_trie rt;
    EXPECT_EQ(0u, rt.count("foo"));
}

TEST(radix_trie, count_increase_after_write) {
    radix_trie rt;
    rt["foo"] = 1;
    EXPECT_EQ(1u, rt.count("foo"));
}
