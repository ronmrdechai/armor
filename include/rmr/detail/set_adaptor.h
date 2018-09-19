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

    insert_return_type insert(node_type&& nh) {
        insert_return_type ret;
        if (nh.empty()) {
            ret.position = end();
        } else {
            iterator it = find(nh.key());
            if (it == end()) {
                ret.position = trie_.reinsert(trie_.root(), nh.key(), nh.ptr_);
                nh.ptr_ = nullptr;
                ret.inserted = true;
            } else {
                ret.node = std::move(nh);
                ret.position = it;
                ret.inserted = false;
            }
        }
        return ret;
    }
    iterator insert(const_iterator hint, node_type&& nh) { 
        if (nh.empty()) return end();
        iterator it = find(nh.key());
        if (it == end()) {
            it = trie_.reinsert(hint, nh.key(), nh.ptr_);
            nh.ptr_ = nullptr;
        }
        return it;
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        size_type pre = size();
        iterator it = emplace_hint(trie_.root(), std::forward<Args>(args)...);
        return { it, size() > pre };
    }
    template <typename... Args>
    iterator emplace_hint(const_iterator hint, Args&&... args ) {
        value_type o(std::forward<Args>(args)...);
        return trie_.emplace(hint, o.first, std::move(o));
    }

    iterator erase(const_iterator pos) { return trie_.erase(pos); }
    iterator erase(iterator pos) { return trie_.erase(pos); }
    iterator erase(const_iterator first, const_iterator last) {
        if (first == begin() && last == end()) clear();
        else while (first != last) erase(first++);
        return remove_const(last);
    }
    size_type erase(const key_type& k) {
        iterator it = find(k);
        if (it == end()) return 0;
        erase(it);
        return 1;
    }

    void swap(set_adaptor& other) noexcept(noexcept(std::declval<Trie>().swap(std::declval<Trie>())))
    { trie_.swap(other.trie_); }

    node_type extract(const_iterator pos) { return node_type(trie_.extract(pos), get_allocator()); }
    node_type extract(const key_type& k) { return extract(find(k)); }

    template <typename _Trie>
    void merge(set_adaptor<T, _Trie>& source) {
        for (auto it = source.begin(), last = source.end(); it != last;) {
            auto pos = it++;
            if (find(pos->first) != end()) continue;
            insert(source.extract(pos));
        }
    }
    template <typename _Trie>
    void merge(set_adaptor<T, _Trie>&& source) { merge(source); }

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
inline bool operator==(const set_adaptor<T, Trie>& x, const set_adaptor<T, Trie>& y)
{ return x.size() == y.size() && std::equal(x.begin(), x.end(), y.begin()); }

template <typename T, typename Trie>
inline bool operator!=(const set_adaptor<T, Trie>& x, const set_adaptor<T, Trie>& y)
{ return !(x == y); }

template <typename T, typename Trie>
inline bool operator<(const set_adaptor<T, Trie>& x, const set_adaptor<T, Trie>& y)
{ return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end()); }

template <typename T, typename Trie>
inline bool operator<=(const set_adaptor<T, Trie>& x, const set_adaptor<T, Trie>& y)
{ return !(y < x); }

template <typename T, typename Trie>
inline bool operator>(const set_adaptor<T, Trie>& x, const set_adaptor<T, Trie>& y)
{ return y < x; }

template <typename T, typename Trie>
inline bool operator>=(const set_adaptor<T, Trie>& x, const set_adaptor<T, Trie>& y)
{ return !(x < y); }

} // namespace rmr::detail
