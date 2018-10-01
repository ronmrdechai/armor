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

#include <rmr/meta.h>
#include <rmr/node_handle.h>

namespace rmr::detail {

template <typename T>
using has_key_mapper = typename T::key_mapper;
template <typename Trie>
std::enable_if_t<is_detected_v<has_key_mapper, Trie>, typename Trie::key_mapper> key_func();

template <typename T>
using has_key_compare = typename T::key_compare;
template <typename Trie>
std::enable_if_t<is_detected_v<has_key_compare, Trie>, typename Trie::key_compare> key_func();

template <typename Trie>
using key_func_t = decltype(key_func<Trie>());

template <typename Derived, typename Trie>
class adaptor_base {
    using derived_type = Derived;
    using key_functor  = key_func_t<Trie>;
public:
    using key_type               = typename Trie::key_type;
    using char_type              = typename Trie::char_type;
    using value_type             = typename Trie::value_type;
    using size_type              = typename Trie::size_type;
    using difference_type        = typename Trie::difference_type;
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

    adaptor_base() = default;

    explicit adaptor_base(key_functor kf, allocator_type alloc = allocator_type()) :
        trie_(std::move(kf), std::move(alloc)) {}
    explicit adaptor_base(allocator_type alloc) : trie_(std::move(alloc)) {}

    template <typename InputIterator>
    adaptor_base(InputIterator first, InputIterator last,
         key_functor kf = key_functor(), allocator_type alloc = allocator_type()
    ) : adaptor_base(std::move(kf), std::move(alloc)) { insert(first, last); }
    template <typename InputIterator>
    adaptor_base(InputIterator first, InputIterator last, allocator_type alloc) :
        adaptor_base(std::move(alloc)) { insert(first, last); }

    adaptor_base(const adaptor_base&) = default;
    adaptor_base(const adaptor_base& other, allocator_type alloc) : trie_(other, std::move(alloc)) {}

    adaptor_base(adaptor_base&& other) = default;
    adaptor_base(adaptor_base&& other, allocator_type alloc) : trie_(std::move(other), std::move(alloc)) {}

    adaptor_base(std::initializer_list<value_type> ilist,
        key_functor kf = key_functor(),
        allocator_type alloc = allocator_type()
    ) : adaptor_base(ilist.begin(), ilist.end(), std::move(kf), std::move(alloc)) {}
    adaptor_base(std::initializer_list<value_type> ilist, allocator_type alloc) :
        adaptor_base(ilist.begin(), ilist.end(), std::move(alloc)) {}

    adaptor_base& operator=(const adaptor_base&) = default;
    adaptor_base& operator=(adaptor_base&&) noexcept(
        std::allocator_traits<allocator_type>::is_always_equal::value
        && std::is_nothrow_move_assignable<key_functor>::value
    ) = default;

    adaptor_base& operator=(std::initializer_list<value_type> ilist)
    { return *this = adaptor_base(ilist); }

    allocator_type get_allocator() const { return trie_.get_allocator(); }

    iterator begin() noexcept { return trie_.begin(); }
    const_iterator begin() const noexcept { return trie_.begin(); }
    const_iterator cbegin() const noexcept { return trie_.cbegin(); }

    iterator end() noexcept { return trie_.end(); }
    const_iterator end() const noexcept { return trie_.end(); }
    const_iterator cend() const noexcept { return trie_.cend(); }

    reverse_iterator rbegin() noexcept { return trie_.rbegin(); }
    const_reverse_iterator rbegin() const noexcept { return trie_.rbegin(); }
    const_reverse_iterator crbegin() const noexcept { return trie_.crbegin(); }

    reverse_iterator rend() noexcept { return trie_.rend(); }
    const_reverse_iterator rend() const noexcept { return trie_.rend(); }
    const_reverse_iterator crend() const noexcept { return trie_.crend(); }

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
    { return derived().emplace_hint(hint, value); }
    template <typename P, typename = std::enable_if_t<std::is_constructible_v<value_type, P&&>>>
    iterator insert(const_iterator hint, P&& value)
    { return derived().emplace_hint(hint, std::forward<P>(value)); }
    iterator insert(const_iterator hint, value_type&& value)
    { return derived().emplace_hint(hint, std::move(value)); }

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
        iterator it = derived().emplace_hint(trie_.root(), std::forward<Args>(args)...);
        return { it, size() > pre };
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

    void swap(adaptor_base& other) noexcept(noexcept(std::declval<Trie>().swap(std::declval<Trie&>())))
    { trie_.swap(other.trie_); }

    friend void swap(adaptor_base& lhs, adaptor_base& rhs) noexcept(
        noexcept(std::declval<adaptor_base>().swap(std::declval<adaptor_base&>()))
    ) { lhs.swap(rhs); }

    node_type extract(const_iterator pos) { return node_type(trie_.extract(pos), get_allocator()); }
    node_type extract(const key_type& k) { return extract(find(k)); }

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

protected:
    Trie trie_;

private:
    derived_type& derived() { return static_cast<derived_type&>(*this); }
    const derived_type& derived() const { return static_cast<const derived_type&>(*this); }
};

template <typename Derived, typename Trie>
inline bool operator==(const adaptor_base<Derived, Trie>& x, const adaptor_base<Derived, Trie>& y)
{ return x.size() == y.size() && std::equal(x.begin(), x.end(), y.begin()); }

template <typename Derived, typename Trie>
inline bool operator!=(const adaptor_base<Derived, Trie>& x, const adaptor_base<Derived, Trie>& y)
{ return !(x == y); }

template <typename Derived, typename Trie>
inline bool operator<(const adaptor_base<Derived, Trie>& x, const adaptor_base<Derived, Trie>& y)
{ return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end()); }

template <typename Derived, typename Trie>
inline bool operator<=(const adaptor_base<Derived, Trie>& x, const adaptor_base<Derived, Trie>& y)
{ return !(y < x); }

template <typename Derived, typename Trie>
inline bool operator>(const adaptor_base<Derived, Trie>& x, const adaptor_base<Derived, Trie>& y)
{ return y < x; }

template <typename Derived, typename Trie>
inline bool operator>=(const adaptor_base<Derived, Trie>& x, const adaptor_base<Derived, Trie>& y)
{ return !(x < y); }

} // namespace rmr::detail
