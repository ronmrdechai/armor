#include <gtest/gtest.h> 

#include <fstream>
#include <string>

#include <rmr/detail/ternary_search_tree.h>
#include <rmr/detail/word_graph.h>
#include <rmr/detail/trie.h>
#include <rmr/functors.h>

using tst = rmr::detail::ternary_search_tree<int, std::less<char>, std::string, std::allocator<int>>;

using trie = rmr::detail::trie<int, 127, rmr::identity<std::size_t>, std::string, std::allocator<int>>;

using array_list = rmr::detail::array_list<int>;

TEST(scratch, trie) {
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

    write_dot(t.root(), std::ofstream("trie.dot"));

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

TEST(scratch, ternary_search_tree) {
    tst t;
    EXPECT_EQ(0u, t.size());

    t.emplace(t.root(), "foo", 42);
    EXPECT_EQ(42, *t.begin());

    t.emplace(t.root(), "baz", 0);
    EXPECT_EQ(0, *t.begin());

    t.emplace(t.root(), "bar", 44);
    EXPECT_EQ(44, *t.begin());

    EXPECT_EQ(42, *t.find("foo"));
    EXPECT_EQ(0, *t.find("baz"));
    EXPECT_EQ(44, *t.find("bar"));

    EXPECT_EQ(3u, t.size());
    EXPECT_EQ(3u, std::distance(t.begin(), t.end()));

    EXPECT_EQ(t.end(), t.find("poo"));
    t.erase(t.find("foo"));
    EXPECT_EQ(t.end(), t.find("foo"));

    EXPECT_EQ(2u, t.size());
    EXPECT_EQ(2u, std::distance(t.begin(), t.end()));

    auto bar_ptr = t.extract(t.find("bar"));
    EXPECT_EQ(44, *bar_ptr);
    EXPECT_EQ(t.end(), t.find("bar"));

    t.emplace(t.root(), "foo",     1);
    t.emplace(t.root(), "fooqux",  5);
    t.emplace(t.root(), "foobaz",  3);
    t.emplace(t.root(), "fooquux", 4);
    t.emplace(t.root(), "foobar",  2);

    EXPECT_EQ(6u, std::distance(t.begin(), t.end()));
    int i = 0;
    auto it = t.begin();
    do EXPECT_EQ(i++, *it); while(++it != t.end());

    EXPECT_EQ(6u, std::distance(t.rbegin(), t.rend()));
    i = 5;
    auto rit = t.rbegin();
    do EXPECT_EQ(i--, *rit); while(++rit != t.rend());

    write_dot(t.root(), std::ofstream("tst.dot"));

    auto [first, last] = t.prefixed_with("foo");
    EXPECT_EQ(5u, std::distance(first, last));

    EXPECT_EQ(1, *t.longest_match("foo"));
    EXPECT_EQ(1, *t.longest_match("fooba"));
    EXPECT_EQ(2, *t.longest_match("foobar"));
    EXPECT_EQ(2, *t.longest_match("foobarbar"));

    tst copy(t);
    EXPECT_TRUE(std::equal(copy.begin(), copy.end(), t.begin()));

    tst move(std::move(t));
    EXPECT_TRUE(std::equal(move.begin(), move.end(), copy.begin()));
    EXPECT_EQ(0u, t.size());
}

TEST(scratch, array_list) {
    array_list a;
    std::allocator<int> alloc;

    EXPECT_EQ(0u, a.size());
    EXPECT_EQ(0u, a.capacity());

    a.emplace_back(alloc, 1);
    EXPECT_EQ(1u, a.size());
    EXPECT_EQ(1u, a.capacity());
    a.emplace_back(alloc, 2);
    EXPECT_EQ(2u, a.size());
    EXPECT_EQ(2u, a.capacity());
    a.emplace_back(alloc, 3);
    EXPECT_EQ(3u, a.size());
    EXPECT_EQ(4u, a.capacity());
    a.emplace_back(alloc, 4);
    EXPECT_EQ(4u, a.size());
    EXPECT_EQ(4u, a.capacity());

    a.remove_back(alloc);
    EXPECT_EQ(3u, a.size());
    EXPECT_EQ(4u, a.capacity());
    a.remove_back(alloc);
    EXPECT_EQ(2u, a.size());
    EXPECT_EQ(4u, a.capacity());
    a.remove_back(alloc);
    EXPECT_EQ(1u, a.size());
    EXPECT_EQ(2u, a.capacity());
    a.remove_back(alloc);
    EXPECT_EQ(0u, a.size());
    EXPECT_EQ(1u, a.capacity());

    a.clear(alloc);
    for (int i = 0; i < 10; ++i) a.emplace_back(alloc, i);
    for (int i = 0; i < 10; ++i) a.emplace_at(alloc, 3, 42);

    for (int i = 0;  i < 3;  ++i) EXPECT_EQ(a[i], i);
    for (int i = 3;  i < 13; ++i) EXPECT_EQ(a[i], 42);
    for (int i = 13; i < 20; ++i) EXPECT_EQ(a[i], i - 10);
}
