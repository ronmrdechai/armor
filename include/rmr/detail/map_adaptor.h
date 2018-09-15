// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <algorithm>
#include <initializer_list>
#include <stdexcept>
#include <tuple>
#include <utility>

#include <rmr/node_handle.h>

namespace rmr::detail {

template <typename T, typename Trie>
class map_adaptor {
public:
    using key_type               = typename Trie::key_type;
    using mapped_type            = T;
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

    map_adaptor() = default;

    explicit map_adaptor(key_mapper km, allocator_type alloc = allocator_type()) :
        trie_(std::move(km), std::move(alloc)) {}
    explicit map_adaptor(allocator_type alloc) : trie_(std::move(alloc)) {}

    template <typename InputIterator>
    map_adaptor(InputIterator first, InputIterator last,
         key_mapper km = key_mapper(), allocator_type alloc = allocator_type()
    ) : map_adaptor(std::move(km), std::move(alloc)) { insert(first, last); }
    template <typename InputIterator>
    map_adaptor(InputIterator first, InputIterator last, allocator_type alloc) :
        map_adaptor(std::move(alloc)) { insert(first, last); }

    map_adaptor(const map_adaptor&) = default;
    map_adaptor(const map_adaptor& other, allocator_type alloc) : trie_(other, std::move(alloc)) {}

    map_adaptor(map_adaptor&& other) = default;
    map_adaptor(map_adaptor&& other, allocator_type alloc) : trie_(std::move(other), std::move(alloc)) {}

    map_adaptor(std::initializer_list<value_type> ilist,
        key_mapper km = key_mapper(),
        allocator_type alloc = allocator_type()
    ) : map_adaptor(ilist.begin(), ilist.end(), std::move(km), std::move(alloc)) {}
    map_adaptor(std::initializer_list<value_type> ilist, allocator_type alloc) :
        map_adaptor(ilist.begin(), ilist.end(), std::move(alloc)) {}

    map_adaptor& operator=(const map_adaptor&) = default;
    map_adaptor& operator=(map_adaptor&&) noexcept(
        std::allocator_traits<allocator_type>::is_always_equal::value
        && std::is_nothrow_move_assignable<key_mapper>::value
    ) = default;

    map_adaptor& operator=(std::initializer_list<value_type> ilist) { return *this = map_adaptor(ilist); }

    allocator_type get_allocator() const { return trie_.get_allocator(); }

    mapped_type& at(const key_type& k) {
        iterator it = find(k);
        if (it == end()) throw std::out_of_range("rmr::at");
        return it->second;
    }
    const mapped_type& at(const key_type& k) const {
        const_iterator it = find(k);
        if (it == end()) throw std::out_of_range("rmr::at");
        return it->second;
    }

    mapped_type& operator[](const key_type& k) { return try_emplace(k).first->second; }
    mapped_type& operator[](key_type&& k) { return try_emplace(std::move(k)).first->second; }

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

    std::pair<iterator, bool> insert(const value_type& value) {
        return emplace(value);
    }
    template <typename P, typename = std::enable_if_t<std::is_constructible_v<value_type, P&&>>>
    std::pair<iterator, bool> insert(P&& value) {
        return emplace(std::forward<P>(value));
    }
    std::pair<iterator, bool> insert(value_type&& value) {
        return emplace(std::move(value));
    }

    iterator insert(const_iterator hint, const value_type& value)
    { return emplace_hint(hint, value); }
    template <typename P, typename = std::enable_if_t<std::is_constructible_v<value_type, P&&>>>
    iterator insert(const_iterator hint, P&& value)
    { return emplace_hint(hint, std::forward<P>(value)); }
    iterator insert(const_iterator hint, value_type&& value)
    { return emplace_hint(hint, std::move(value)); }

    template <typename InputIt>
    void insert(InputIt first, InputIt last) { while (first != last) insert(*(first++)); }
    void insert(std::initializer_list<value_type> ilist) { insert(ilist.begin(), ilist.end()); }

    insert_return_type insert(node_type&& nh); // TODO
    iterator insert(const_iterator hint, node_type&& nh); // TODO

    template <typename M>
    std::pair<iterator, bool> insert_or_assign(const key_type& k, M&& obj) {
        const_iterator it = find(k);
        if (it == end()) return insert({ k, std::forward<M>(obj) });

        it->second = std::forward<M>(obj);
        return { it, false };
    }
    template <typename M>
    std::pair<iterator, bool> insert_or_assign(key_type&& k, M&& obj) {
        const_iterator it = find(k);
        if (it == end()) return insert({ std::move(k), std::forward<M>(obj) });

        it->second = std::forward<M>(obj);
        return { it, false };
    }

    template <typename M>
    iterator insert_or_assign(const_iterator hint, const key_type& k, M&& obj) {
        const_iterator it = find(k);
        if (it == end()) return insert({ hint, k, std::forward<M>(obj) });

        it->second = std::forward<M>(obj);
        return { it, false };
    }
    template <typename M>
    iterator insert_or_assign(const_iterator hint, key_type&& k, M&& obj) {
        const_iterator it = find(k);
        if (it == end()) return insert({ hint, std::move(k), std::forward<M>(obj) });

        it->second = std::forward<M>(obj);
        return { it, false };
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        size_type pre = size();
        const_iterator it = emplace_hint(trie_.root(), std::forward<Args>(args)...);
        return { it, size() > pre };
    }
    template <typename... Args>
    iterator emplace_hint(const_iterator hint, Args&&... args ) {
        value_type o(std::forward<Args>(args)...);
        return trie_.emplace(hint, o.first, std::move(o));
    }
    template <typename... Args>
    std::pair<iterator, bool> try_emplace(const key_type& k, Args&&... args) {
        iterator it = find(k);
        if (it == end()) return emplace(std::piecewise_construct,
            std::forward_as_tuple(k), std::forward_as_tuple(std::forward<Args>(args)...)
        );
        return { it, false };
    }
    template <typename... Args>
    std::pair<iterator, bool> try_emplace(key_type&& k, Args&&... args) {
        iterator it = find(k);
        if (it == end()) return emplace(std::piecewise_construct,
            std::forward_as_tuple(std::move(k)), std::forward_as_tuple(std::forward<Args>(args)...)
        );
        return { it, false };
    }
    template <typename... Args>
    iterator try_emplace(const_iterator hint, const key_type& k, Args&&... args) {
        iterator it = find(k);
        if (it == end()) return emplace_hint(hint, std::piecewise_construct,
            std::forward_as_tuple(k), std::forward_as_tuple(std::forward<Args>(args)...)
        );
        return it;
    }
    template <typename... Args>
    iterator try_emplace(const_iterator hint, key_type&& k, Args&&... args) {
        iterator it = find(k);
        if (it == end()) return emplace_hint(hint, std::piecewise_construct,
            std::forward_as_tuple(std::move(k)), std::forward_as_tuple(std::forward<Args>(args)...)
        );
        return it;
    }

    iterator erase(const_iterator pos) { return trie_.erase(pos); }
    iterator erase(iterator pos) { return trie_.erase(pos); }
    iterator erase(const_iterator first, const_iterator last) {
        if (first == begin() && last == end()) clear();
        else while (first != last) erase(first++);
        return remove_const(last);
    }
    size_type erase(const key_type& k) {
        const_iterator it = find(k);
        if (it == end()) return 0;
        erase(it);
        return 1;
    }

    void swap(map_adaptor& other) noexcept(noexcept(std::declval<Trie>().swap(std::declval<Trie>())))
    { trie_.swap(other.trie_); }

    node_type extract(const_iterator position); // TODO
    node_type extract(const key_type& k); // TODO
    template <typename _Trie> void merge(map_adaptor<T, _Trie>& source); // TODO
    template <typename _Trie> void merge(map_adaptor<T, _Trie>&& source); // TODO

    size_type count(const key_type& k) const { return find(k) != end(); }

    iterator find(const key_type& k) { return trie_.find(k); }
    const_iterator find(const key_type& k) const { return trie_.find(k); }

    std::pair<iterator, iterator> prefixed_with(const key_type& k)
    { return trie_.prefixed_with(k); }
    std::pair<const_iterator, const_iterator> prefixed_with(const key_type& k) const
    { return trie_.prefixed_with(k); }

    iterator longest_match(const key_type& k)
    { return trie_.longest_match(k); }
    const_iterator longest_match(const key_type& k) const
    { return trie_.longest_match(k); }

    key_mapper key_map() const { return trie_.key_map(); } 

private:
    Trie trie_;
};

template <typename T, typename Trie>
inline bool operator==(const map_adaptor<T, Trie>& x, const map_adaptor<T, Trie>& y)
{ return x.size() == y.size() && std::equal(x.begin(), x.end(), y.begin()); }

template <typename T, typename Trie>
inline bool operator!=(const map_adaptor<T, Trie>& x, const map_adaptor<T, Trie>& y)
{ return !(x == y); }

template <typename T, typename Trie>
inline bool operator<(const map_adaptor<T, Trie>& x, const map_adaptor<T, Trie>& y)
{ return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end()); }

template <typename T, typename Trie>
inline bool operator<=(const map_adaptor<T, Trie>& x, const map_adaptor<T, Trie>& y)
{ return !(y < x); }

template <typename T, typename Trie>
inline bool operator>(const map_adaptor<T, Trie>& x, const map_adaptor<T, Trie>& y)
{ return y < x; }

template <typename T, typename Trie>
inline bool operator>=(const map_adaptor<T, Trie>& x, const map_adaptor<T, Trie>& y)
{ return !(x < y); }

} // namespace rmr::detail
