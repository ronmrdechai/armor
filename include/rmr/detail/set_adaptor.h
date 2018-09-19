// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <initializer_list>
#include <stdexcept>
#include <tuple>
#include <utility>

#include <rmr/node_handle.h>

namespace rmr::detail {

template <typename T, typename Trie>
class set_adaptor {
public:
    using key_type               = typename Trie::key_type;
    using char_type              = typename Trie::char_type;
    using value_type             = typename Trie::value_type;
    using size_type              = typename Trie::size_type;
    using difference_type        = typename Trie::difference_type;
    using key_mapper             = typename Trie::key_mapper;
    using allocator_type         = typename Trie::allocator_type;
    using reference              = typename Trie::reference;
    using const_reference        = typename Trie::const_reference;
    using pointer                = typename Trie::pointer;
    using const_pointer          = typename Trie::const_pointer;
    using iterator               = typename Trie::iterator;
    using const_iterator         = typename Trie::const_iterator;
    using reverse_iterator       = typename Trie::reverse_iterator;
    using const_reverse_iterator = typename Trie::const_reverse_iterator;

    using node_type          = node_handle<key_type, value_type, allocator_type>;
    using insert_return_type = node_insert_return<iterator, node_type>;

    set_adaptor() = default;

    explicit set_adaptor(key_mapper km, allocator_type alloc = allocator_type()) :
        trie_(std::move(km), std::move(alloc)) {}
    explicit set_adaptor(allocator_type alloc) : trie_(std::move(alloc)) {}

    template <typename InputIterator>
    set_adaptor(InputIterator first, InputIterator last,
         key_mapper km = key_mapper(), allocator_type alloc = allocator_type()
    ) : set_adaptor(std::move(km), std::move(alloc)) { insert(first, last); }
    template <typename InputIterator>
    set_adaptor(InputIterator first, InputIterator last, allocator_type alloc) :
        set_adaptor(std::move(alloc)) { insert(first, last); }

    set_adaptor(const set_adaptor&) = default;
    set_adaptor(const set_adaptor& other, allocator_type alloc) : trie_(other, std::move(alloc)) {}

    set_adaptor(set_adaptor&& other) = default;
    set_adaptor(set_adaptor&& other, allocator_type alloc) : trie_(std::move(other), std::move(alloc)) {}

    set_adaptor(std::initializer_list<value_type> ilist,
        key_mapper km = key_mapper(),
        allocator_type alloc = allocator_type()
    ) : set_adaptor(ilist.begin(), ilist.end(), std::move(km), std::move(alloc)) {}
    set_adaptor(std::initializer_list<value_type> ilist, allocator_type alloc) :
        set_adaptor(ilist.begin(), ilist.end(), std::move(alloc)) {}

    set_adaptor& operator=(const set_adaptor&) = default;
    set_adaptor& operator=(set_adaptor&&) noexcept(
        std::allocator_traits<allocator_type>::is_always_equal::value
        && std::is_nothrow_move_assignable<key_mapper>::value
    ) = default;

    set_adaptor& operator=(std::initializer_list<value_type> ilist) { return *this = set_adaptor(ilist); }

    allocator_type get_allocator() const { return trie_.get_allocator(); }

    iterator begin() noexcept { return trie_.begin(); }
    const_iterator begin() const noexcept { return trie_.begin(); }
    const_iterator cbegin() const noexcept { return trie_.cbegin(); }

    iterator end() noexcept { return trie_.end(); }
    const_iterator end() const noexcept { return trie_.end(); }
    const_iterator cend() const noexcept { return trie_.cend(); }

    iterator rbegin() noexcept { return trie_.rbegin(); }
    const_iterator rbegin() const noexcept { return trie_.rbegin(); }
    const_iterator crbegin() const noexcept { return trie_.crbegin(); }

    iterator rend() noexcept { return trie_.rend(); }
    const_iterator rend() const noexcept { return trie_.rend(); }
    const_iterator crend() const noexcept { return trie_.crend(); }

    [[nodiscard]] bool empty() const noexcept { return size() == 0; }
    size_type size() const noexcept { return trie_.size(); }
    size_type max_size() const noexcept { return std::numeric_limits<size_type>::max(); }

    void clear() noexcept { trie_.clear(); }

private:
    Trie trie_;
};

} // namespace rmr::detail
