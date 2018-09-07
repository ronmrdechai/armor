#include <gtest/gtest.h>

#include <string>

#include <rmr/functors.h>
#include <rmr/detail/prefix_tree.h>

using prefix_tree = rmr::detail::prefix_tree<
    int, 127, rmr::identity<std::size_t>, std::string, std::allocator<int>
>;

TEST(prefix_tree, scratch) {
    prefix_tree p;
}
