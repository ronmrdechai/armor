// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <rmr/detail/util.h>
#include <rmr/detail/trie_node.h>

namespace rmr::detail {

template <typename T, typename Char>
struct tst_node : trie_node<T, 3> {
    Char c;
};

template <typename T, typename Compare, typename Key, typename Allocator>
class ternary_search_tree {
    using alloc_traits        = std::allocator_traits<Allocator>;
public:
    using key_type               = Key;
    using char_type              = typename key_type::value_type;
    using value_type             = T;
    using size_type              = std::size_t;
    using difference_type        = std::ptrdiff_t;
    using key_compare            = Compare;
    using allocator_type         = Allocator;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using pointer                = typename alloc_traits::pointer;
    using const_pointer          = typename alloc_traits::const_pointer;
private:
    using node_type           = tst_node<T, char_type>;
    using node_allocator_type = typename alloc_traits::template rebind_alloc<node_type>;
    using node_alloc_traits   = typename alloc_traits::template rebind_traits<node_type>;
public:
    using iterator               = trie_iterator<3, node_type*>;
    using const_iterator         = trie_iterator<3, const node_type*>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // interface...
};

} // namespace rmr::detail
