// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <rmr/detail/util.h>
#include <rmr/detail/trie_node_base.h>

namespace rmr::detail {

template <typename T, typename Char>
struct tst_node : trie_node_base<tst_node<T, Char>, T, 3> { Char c; };

template <typename T, typename Compare, typename Key, typename Allocator>
class ternary_search_tree {
    static constexpr std::size_t R = 3;
    using alloc_traits = std::allocator_traits<Allocator>;
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
    using iterator               = trie_iterator<R, node_type*>;
    using const_iterator         = trie_iterator<R, const node_type*>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    ternary_search_tree() = default;
    explicit ternary_search_tree(allocator_type alloc) : impl_(node_allocator_type(std::move(alloc))) {}
    explicit ternary_search_tree(key_compare kc, allocator_type alloc) :
        impl_(std::move(kc), node_allocator_type(std::move(alloc)))
    {}
    ~ternary_search_tree() { clear(); }

    template <typename... Args>
    iterator emplace(const_iterator pos, const key_type& key, Args&&... args)
    { return emplace(remove_const(pos), key, std::forward<Args>(args)...); }
    template <typename... Args>
    iterator emplace(iterator pos, const key_type& key, Args&&... args)
    { return insert_node(pos.node, key, make_value(get_allocator(), std::forward<Args>(args)...)); }

    iterator root() noexcept { return remove_const(croot()); }
    const_iterator root() const noexcept { return croot(); }
    const_iterator croot() const noexcept { return &impl_.root; }

    iterator begin() noexcept { return remove_const(cbegin()); }
    const_iterator begin() const noexcept { return cbegin(); }
    const_iterator cbegin() const noexcept { return ++const_iterator(&impl_.base); }

    iterator end() noexcept { return remove_const(cend()); }
    const_iterator end() const noexcept { return cend(); }
    const_iterator cend() const noexcept { return &impl_.base; }

    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }

    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

    size_type size() const noexcept { return impl_.size; }

    void clear() noexcept {
        auto value_alloc = get_allocator();
        auto& node_alloc = get_node_allocator();
        clear_node(&impl_.root, node_alloc, value_alloc);
        impl_.size = 0;
    }

    allocator_type get_allocator() const { return get_node_allocator(); }
    key_compare    key_comp()      const { return impl_; }

private:
    node_type* insert_node(const node_type* hint, const key_type& key, pointer v) {
        (void)hint, (void)key, (void)v;
        return nullptr;
    }

    const auto& get_node_allocator() const { return impl_; }
          auto& get_node_allocator()       { return impl_; }

    struct tst_header {
        node_type base;
        node_type root;
        size_type size;

        tst_header() { reset(); }
        tst_header& operator=(tst_header&& other) {
            for (size_type i = 0; i < R; ++i) {
                root.children[i] = other.root.children[i];
                if (root.children[i] != nullptr) root.children[i]->parent = &root;
            }
            size = other.size;
            other.reset();
            return *this;
        }

        void reset() {
            std::fill(std::begin(base.children), std::end(base.children), nullptr);
            base.children[0] = &root;
            base.parent_index = R;
            base.value = nullptr;

            std::fill(std::begin(root.children), std::end(root.children), nullptr);
            root.parent = &base;
            root.parent_index = 0;
            root.value = nullptr;

            size = 0;
        }
    };

    struct tst_impl : tst_header, key_compare, node_allocator_type {
        tst_impl() = default;
        tst_impl(node_allocator_type alloc) :
            tst_header(), key_compare(), node_allocator_type(std::move(alloc))
        {}
        tst_impl(key_compare kc, node_allocator_type alloc) :
            tst_header(), key_compare(std::move(kc)), node_allocator_type(std::move(alloc))
        {}
        tst_impl& operator=(tst_impl&&) = default;
    };

    tst_impl impl_;
};

} // namespace rmr::detail
