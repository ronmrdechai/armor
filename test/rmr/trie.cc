#include <gtest/gtest.h>

#include <string>

#include <rmr/functors.h>
#include <rmr/detail/trie.h>

using trie = rmr::detail::trie<
    int, 127, rmr::identity<std::size_t>, std::string, std::allocator<int>
>;

TEST(trie, scratch) {
    trie t;
    EXPECT_EQ(t.begin(), t.end());

    t.emplace(t.root(), "hello", 7);
    EXPECT_EQ(7, *t.begin());

    EXPECT_EQ(1, std::distance(t.begin(), t.end()));

    EXPECT_EQ(7, *t.find("hello"));
    EXPECT_EQ(t.end(), t.find("bye"));

    t.emplace(t.root(), "foo", 1);
    t.emplace(t.root(), "bar", 6);
    EXPECT_EQ(1, *t.find("foo"));
    EXPECT_EQ(6, *t.find("bar"));
    auto it = t.erase(t.find("foo"));
    EXPECT_EQ(it, t.find("hello"));
    EXPECT_EQ(t.end(), t.find("foo"));

    EXPECT_EQ(t.find("bar"), t.longest_match("barbar"));

    t.emplace(t.root(), "aa", 0);
    t.emplace(t.root(), "aaa", 1); t.emplace(t.root(), "aab", 2);
    t.emplace(t.root(), "aac", 3); t.emplace(t.root(), "aad", 4);
    t.emplace(t.root(), "ab", 5);
    auto [first, last] = t.prefixed_with("aa");
    EXPECT_EQ(5, std::distance(first, last));

    std::vector<int> v(t.begin(), t.end());
    std::vector<int> r(t.rbegin(), t.rend());
    EXPECT_EQ(8u, v.size());
    EXPECT_EQ(8u, r.size());

    std::reverse(v.begin(), v.end());
    EXPECT_EQ(r, v);

    trie copy(t);
    EXPECT_EQ(8u, t.size());
    EXPECT_EQ(8u, copy.size());
    std::vector<int> tv(   t.begin(),    t.end());
    std::vector<int> cv(copy.begin(), copy.end());
    EXPECT_EQ(tv, cv);

    t.clear();
    EXPECT_EQ(0u, t.size());
    EXPECT_EQ(0u, std::distance(t.begin(), t.end()));
    EXPECT_EQ(t.end(), t.find("foo"));

    cv = std::vector<int>(copy.begin(), copy.end());
    EXPECT_EQ(tv, cv);

    trie move(std::move(copy));

    EXPECT_EQ(8u, move.size());
    std::vector<int> mv(move.begin(), move.end());
    EXPECT_EQ(0u, copy.size());
    EXPECT_EQ(copy.end(), copy.find("foo"));
    EXPECT_EQ(tv, mv);

    trie assign;
    assign = move;
    EXPECT_EQ(8u, assign.size());
    std::vector<int> av(assign.begin(), assign.end());
    EXPECT_EQ(mv, av);

    trie swap;
    swap.emplace(swap.root(), "thing", 1);
    swap.emplace(swap.root(), "stuff", 2);
    assign.swap(swap);

    EXPECT_EQ(8u, swap.size());
    EXPECT_EQ(8u, std::distance(swap.begin(), swap.end()));
    EXPECT_EQ(0, *swap.find("aa"));
    EXPECT_EQ(1, *swap.find("aaa"));

    EXPECT_EQ(2u, assign.size());
    EXPECT_EQ(2u, std::distance(assign.begin(), assign.end()));
    EXPECT_EQ(1, *assign.find("thing"));
    EXPECT_EQ(2, *assign.find("stuff"));

    std::swap(assign, swap);
    EXPECT_EQ(8u, assign.size());
    EXPECT_EQ(8u, std::distance(assign.begin(), assign.end()));
    EXPECT_EQ(0, *assign.find("aa"));
    EXPECT_EQ(1, *assign.find("aaa"));

    EXPECT_EQ(2u, swap.size());
    EXPECT_EQ(2u, std::distance(swap.begin(), swap.end()));
    EXPECT_EQ(1, *swap.find("thing"));
    EXPECT_EQ(2, *swap.find("stuff"));
}
