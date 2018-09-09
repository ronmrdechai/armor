#include <gtest/gtest.h>

#include <string>

#include <rmr/functors.h>
#include <rmr/detail/prefix_tree.h>

using prefix_tree = rmr::detail::prefix_tree<
    int, 127, rmr::identity<std::size_t>, std::string, std::allocator<int>
>;

TEST(prefix_tree, scratch) {
    prefix_tree p;
    EXPECT_EQ(p.begin(), p.end());

    p.emplace(p.root(), "hello", 42);
    EXPECT_EQ(42, *p.begin());

    EXPECT_EQ(1, std::distance(p.begin(), p.end()));

    EXPECT_EQ(42, *p.find("hello"));
    EXPECT_EQ(p.end(), p.find("bye"));

    p.emplace(p.root(), "foo", 1);
    p.emplace(p.root(), "bar", 2);
    EXPECT_EQ(1, *p.find("foo"));
    EXPECT_EQ(2, *p.find("bar"));
    auto it = p.erase(p.find("foo"));
    EXPECT_EQ(it, p.find("hello"));
    EXPECT_EQ(p.end(), p.find("foo"));

    EXPECT_EQ(p.find("bar"), p.longest_match("barbar"));

    p.emplace(p.root(), "aaa", 1); p.emplace(p.root(), "aab", 2);
    p.emplace(p.root(), "aac", 3); p.emplace(p.root(), "aad", 4);
    auto [first, last] = p.prefixed_with("aa");
    EXPECT_EQ(4, std::distance(first, last));
}
