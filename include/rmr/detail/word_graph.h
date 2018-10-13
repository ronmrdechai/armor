// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <algorithm>

#include <rmr/detail/util.h>
#include <rmr/detail/trie_node_base.h>

namespace rmr::detail {

template <typename T>
class array_list {
public:
    using size_type = std::size_t;
    using iterator = T*;
    using const_iterator = const T*;
    using reference = T&;
    using const_reference = const T&;

    reference operator[](size_type n)             { return data_[n]; }
    const_reference operator[](size_type n) const { return data_[n]; }

    iterator begin() { return &data_[0]; }
    const_iterator begin() const { return &data_[0]; }
    const_iterator cbegin() const { return &data_[0]; }
    iterator end() { return &data_[size_]; }
    const_iterator end() const { return &data_[size_]; }
    const_iterator cend() const { return &data_[size_]; }

    size_type size() const { return size_; }

    template <typename Allocator, typename... Args>
    void emplace_at(size_type index, Allocator& alloc, Args&&... args);
    template <typename Allocator>
    void remove_at(size_type index, Allocator& alloc);

    template <typename Allocator, typename... Args>
    void emplace_back(Allocator& alloc, Args&&... args);
    template <typename Allocator>
    void remove_back(Allocator& alloc);

private:
    T*        data_;
    size_type size_;
    size_type capacity_;
};

template <typename Char>
struct string_view {
    const Char* data;
    std::size_t size;
};

template <typename T, std::size_t R>
struct word_graph_node : trie_node_base<word_graph_node<T, R>, T, R> {};

template <typename T, std::size_t R, typename KeyMapper, typename Key, typename Allocator, typename Storage>
class word_graph {
    using alloc_traits        = std::allocator_traits<Allocator>;
    using node_type           = word_graph_node<T, R>;
    using node_allocator_type = typename alloc_traits::template rebind_alloc<node_type>;
    using node_alloc_traits   = typename alloc_traits::template rebind_traits<node_type>;
public:
    using key_type               = Key;
    using char_type              = typename key_type::value_type;
    using value_type             = T;
    using size_type              = std::size_t;
    using difference_type        = std::ptrdiff_t;
    using key_mapper             = KeyMapper;
    using allocator_type         = Allocator;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using pointer                = typename alloc_traits::pointer;
    using const_pointer          = typename alloc_traits::const_pointer;
    using iterator               = typename Storage::iterator;
    using const_iterator         = typename Storage::const_iterator;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
};

} // namespace rmr::detail
