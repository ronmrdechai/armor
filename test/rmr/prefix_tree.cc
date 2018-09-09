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

    EXPECT_EQ(std::distance(p.begin(), p.end()), 1);
}
