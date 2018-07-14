#include <gtest/gtest.h>

#include <moat/trie.h>

using trie = moat::trie<int, 127>;

TEST(trie, insertion_and_access) {
    trie t;
    t["foo"] = 1;
    t["bar"] = 2;
    EXPECT_EQ(1, t["foo"]);
    EXPECT_EQ(2, t["bar"]);
}

TEST(trie, safe_access_read) {
    trie t;
    t["foo"] = 1;
    t["bar"] = 2;
    EXPECT_EQ(1, t.at("foo"));
    EXPECT_EQ(2, t.at("bar"));
}

TEST(trie, safe_access_write) {
    trie t;
    t["foo"] = 1;
    t.at("foo") = 2;
    EXPECT_EQ(2, t["foo"]);
}

TEST(trie, safe_access_throws) {
    trie t;

    ASSERT_THROW(t.at("foo"), std::out_of_range);
    ASSERT_THROW(t.at("foo") = 1, std::out_of_range);
}

TEST(trie, default_is_empty) {
    trie t;
    EXPECT_TRUE(t.empty());
}

TEST(trie, not_empty_after_insert) {
    trie t;
    t["foo"] = 1;
    EXPECT_FALSE(t.empty());
}

TEST(trie, empty_after_clear) {
    trie t;
    t["foo"] = 1;
    t.clear();
    EXPECT_TRUE(t.empty());
}

TEST(trie, default_size_is_zero) {
    trie t;
    EXPECT_EQ(0u, t.size());
}

TEST(trie, size_increase_after_write) {
    trie t;
    t["foo"] = 1;
    EXPECT_EQ(1u, t.size());
    t["bar"] = 1;
    EXPECT_EQ(2u, t.size());
}

TEST(trie, default_count_is_zero) {
    trie t;
    EXPECT_EQ(0u, t.count("foo"));
}

TEST(trie, count_increase_after_write) {
    trie t;
    t["foo"] = 1;
    EXPECT_EQ(1u, t.count("foo"));
}

TEST(trie, read_iteration) {
    trie t;
    t["foo"] = 1;
    t["bar"] = 2;
    t["baz"] = 3;
    for (auto& [key, value] : t) {
        std::cout << key << "=" << value << std::endl;
    }
}
