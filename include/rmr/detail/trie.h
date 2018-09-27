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

template <typename T, std::size_t R>
struct trie_node : trie_node_base<trie_node<T, R>, T, R> {};

template <typename T, std::size_t R, typename KeyMapper, typename Key, typename Allocator>
class trie {
    using alloc_traits        = std::allocator_traits<Allocator>;
    using node_type           = trie_node<T, R>;
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
    using iterator               = trie_iterator<R, node_type*>;
    using const_iterator         = trie_iterator<R, const node_type*>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    trie() = default;
    explicit trie(allocator_type alloc) : impl_(node_allocator_type(std::move(alloc))) {}
    explicit trie(key_mapper km, allocator_type alloc) :
        impl_(std::move(km), node_allocator_type(std::move(alloc)))
    {}

    trie(const trie& other) : trie(
        other.key_map(),
        alloc_traits::select_on_container_copy_construction(other.get_allocator())
    ) {
        copy_nodes(&other.impl_.root, &impl_.root, nullptr);
        impl_.size = other.impl_.size;
    }
    trie(const trie& other, allocator_type alloc) : trie(other.key_map(), std::move(alloc)) {
        copy_nodes(&other.impl_.root, &impl_.root, nullptr);
        impl_.size = other.impl_.size;
    }

    trie(trie&& other) : trie(std::move(other.key_map()), std::move(other.get_allocator())) {
        static_cast<trie_header&>(impl_) = std::move(static_cast<trie_header&>(other.impl_));
    }
    trie(trie&& other, allocator_type alloc) : trie(std::move(other.key_map()), std::move(alloc)) {
        if (alloc != other.get_allocator()) {
            auto other_alloc = other.get_allocator();
            move_nodes(other_alloc, &other.impl_.root, &impl_.root, nullptr);
            other.clear();
        } else {
            static_cast<trie_header&>(impl_) = std::move(static_cast<trie_header&>(other.impl_));
        }
    }
    ~trie() { clear(); }

    trie& operator=(const trie& other) {
        clear();
        impl_.size = other.impl_.size;
        static_cast<key_mapper&>(impl_) = other.key_map();
        if (alloc_traits::propagate_on_container_copy_assignment::value) {
            static_cast<node_allocator_type&>(impl_) = other.get_node_allocator();
        }
        copy_nodes(&other.impl_.root, &impl_.root, nullptr);
        return *this;
    }

    trie& operator=(trie&& other) noexcept(
        alloc_traits::is_always_equal::value && std::is_nothrow_move_assignable<key_mapper>::value
    ) {
        clear();
        static_cast<key_mapper&>(impl_) = other.key_map();
        if (alloc_traits::propagate_on_container_move_assignment::value)
            static_cast<node_allocator_type&>(impl_) = other.get_node_allocator();

        auto other_alloc = other.get_allocator();
        if (!alloc_traits::propagate_on_container_move_assignment::value &&
                get_allocator() != other.get_allocator()) {
            move_nodes(other_alloc, &other.impl_.root, &impl_.root, nullptr);
            impl_.size = other.impl_.size;
        } else { static_cast<trie_header&>(impl_) = std::move(static_cast<trie_header&>(other.impl_)); }

        other.clear();
        return *this;
    }

    void swap(trie& other) noexcept(
        alloc_traits::is_always_equal::value && std::is_nothrow_swappable<key_mapper>::value
    ) {
        if (alloc_traits::propagate_on_container_swap::value) {
            node_allocator_type& this_alloc = impl_;
            static_cast<node_allocator_type&>(impl_) = other.get_node_allocator();
            other.get_node_allocator() = this_alloc;
        }
        key_mapper this_key_map = key_map();
        static_cast<key_mapper&>(impl_) = other.key_map();
        static_cast<key_mapper&>(other.impl_) = this_key_map;
        for (size_type i = 0; i < R; ++i) {
            std::swap(impl_.root.children[i], other.impl_.root.children[i]);
            if (impl_.root.children[i] != nullptr)
                impl_.root.children[i]->parent = &impl_.root;
            if (other.impl_.root.children[i] != nullptr)
                other.impl_.root.children[i]->parent = &other.impl_.root;
        }
        std::swap(impl_.size, other.impl_.size);
    }

    template <typename... Args>
    iterator emplace(const_iterator pos, const key_type& key, Args&&... args)
    { return emplace(remove_const(pos), key, std::forward<Args>(args)...); }
    template <typename... Args>
    iterator emplace(iterator pos, const key_type& key, Args&&... args)
    { return insert_node(pos.node, key, make_value(get_allocator(), std::forward<Args>(args)...)); }

    iterator reinsert(const_iterator pos, const key_type& key, const_pointer p)
    { return reinsert(remove_const(pos), key, const_cast<pointer>(p)); }
    iterator reinsert(iterator pos, const key_type& key, pointer p)
    { return insert_node(pos.node, key, p); }

    iterator find(const key_type& key)
    { return remove_const(const_cast<const trie&>(*this).find(key)); }
    const_iterator find(const key_type& key) const
    { return find_key(&impl_.root, key.begin(), key.end()); }

    iterator erase(const_iterator pos) { return erase(remove_const(pos)); }
    iterator erase(iterator pos) {
        iterator next = std::next(pos);
        erase_node(pos.node);
        impl_.size -= 1;
        return next;
    }

    pointer extract(const_iterator pos) { return extract_value(remove_const(pos).node); }

    iterator longest_match(const key_type& key)
    { return remove_const(const_cast<const trie&>(*this).longest_match(key)); }
    const_iterator longest_match(const key_type& key) const
    { return longest_match(&impl_.root, key.begin(), key.end()); }

    std::pair<iterator, iterator> prefixed_with(const key_type& key) {
        auto p = const_cast<const trie&>(*this).prefixed_with(key);
        return { remove_const(p.first), remove_const(p.second) };
    }
    std::pair<const_iterator, const_iterator>
    prefixed_with(const key_type& key) const {
        const_iterator first = find_key_unsafe(&impl_.root, key.begin(), key.end());
        if (first == end()) return { end(), end() };
        auto last = skip(first);

        if (                first.node->value == nullptr) ++first;
        if (last != end() && last.node->value == nullptr) ++last;
        return {first, last};
    }

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
    key_mapper     key_map()       const { return impl_; }

private:
    node_type* copy_nodes(const node_type* src, node_type* dst, node_type* parent) {
        if (dst == nullptr) dst = make_node(parent, src->parent_index);

        if (src->value != nullptr) dst->value = make_value(get_allocator(), *src->value);
        for (size_type i = 0; i < R; ++i) {
            if (src->children[i] != nullptr)
                dst->children[i] = copy_nodes(src->children[i], dst->children[i], dst);
        }
        return dst;
    }

    node_type* move_nodes(allocator_type& alloc, node_type* src, node_type* dst, node_type* parent) {
        if (dst == nullptr) dst = make_node(parent, src->parent_index);

        if (src->value != nullptr) {
            dst->value = make_value(get_allocator(), std::move(*src->value));
            destroy_and_deallocate(alloc, src->value);
            src->value = nullptr;
        }
        for (size_type i = 0; i < R; ++i) {
            if (src->children[i] != nullptr)
                dst->children[i] = move_nodes(alloc, src->children[i], dst->children[i], dst);
        }
        return dst;
    }

    node_type* insert_node(const node_type* hint, const key_type& key, pointer v) {
        size_type rank = 0;
        const node_type* cur = hint;
        while (cur != &impl_.root) { ++rank; cur = cur->parent; }
        return insert_node(hint, rank, key, v);
    }
    node_type* insert_node(const node_type* _hint, size_type rank, const key_type& key, pointer v) {
        node_type* hint = const_cast<node_type*>(_hint);
        node_type* ret;
        insert_node(hint, hint->parent, hint->parent_index, key, rank, v, ret);
        return ret;
    }
    node_type* insert_node(
        node_type* root,
        node_type* parent,
        size_type parent_index,
        const key_type& key,
        size_type index,
        pointer v,
        node_type*& ret
    ) {
        if (root == nullptr) root = make_node(parent, parent_index);
        if (index == key.size()) {
            if (root->value == nullptr) { root->value = v; ++impl_.size; }
            else { auto alloc = get_allocator(); destroy_and_deallocate(alloc, v); }
            ret = root;
        } else {
            size_type parent_index = key_map()(key[index]);
            auto& child = root->children[parent_index];
            child = insert_node(child, root, parent_index, key, index + 1, v, ret);
        }
        return root;
    }

    const auto& get_node_allocator() const { return impl_; }
          auto& get_node_allocator()       { return impl_; }

    node_type* make_node(node_type* parent, size_type parent_index) {
        auto& node_alloc = get_node_allocator();
        node_type* n = node_alloc_traits::allocate(node_alloc, 1);
        node_alloc_traits::construct(node_alloc, n);

        std::fill(std::begin(n->children), std::end(n->children), nullptr);
        n->parent = parent;
        n->parent_index = parent_index;
        n->value = nullptr;

        return n;
    }

    const node_type* find_key(
        const node_type* root,
        typename key_type::const_iterator cur,
        typename key_type::const_iterator last
    ) const {
        const node_type* pos = find_key_unsafe(root, cur, last);
        if (pos->value == nullptr) return &impl_.base;
        return pos;
    }

    const node_type* find_key_unsafe(
        const node_type* root,
        typename key_type::const_iterator cur,
        typename key_type::const_iterator last
    ) const {
        if (root == nullptr) return &impl_.base;
        if (cur == last)     return root;

        root = root->children[key_map()(*cur)];
        return find_key_unsafe(root, ++cur, last);
    }

    void erase_node(node_type* node) {
        auto value_alloc = get_allocator();
        auto& node_alloc = get_node_allocator();

        delete_node_value(node, value_alloc);
        if (children_count(node) == 0) {
            node_type* parent = node->parent;
            while (children_count(parent) == 1 && parent != &impl_.root && parent->value == nullptr) {
                node   = node->parent;
                parent = node->parent;
            }
            unlink(node);
            clear_node(node, node_alloc, value_alloc);
        }
    }

    pointer extract_value(node_type* node) {
        pointer v(std::move(node->value));
        node->value = nullptr;
        impl_.size--;
        return v;
    }

    const node_type* longest_match(
        const node_type* root,
        typename key_type::const_iterator cur,
        typename key_type::const_iterator last
    ) const {
        auto pos = longest_match_candidate(root, root->parent, cur, last);
        while (pos->value == nullptr && pos->parent_index != R) pos = pos->parent;
        return pos;
    }
    const node_type* longest_match_candidate(
        const node_type* node, const node_type* prev,
        typename key_type::const_iterator cur,
        typename key_type::const_iterator last
    ) const {
        if (node == nullptr) return prev;
        if (cur == last)     return node;
        prev = node;
        node = node->children[key_map()(*cur)];
        return longest_match_candidate(node, prev, ++cur, last);
    }

    struct trie_header {
        node_type base;
        node_type root;
        size_type size;

        trie_header() { reset(); }
        trie_header& operator=(trie_header&& other) {
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

    struct trie_impl : trie_header, key_mapper, node_allocator_type {
        trie_impl() = default;
        trie_impl(node_allocator_type alloc) :
            trie_header(), key_mapper(), node_allocator_type(std::move(alloc))
        {}
        trie_impl(key_mapper km, node_allocator_type alloc) :
            trie_header(), key_mapper(std::move(km)), node_allocator_type(std::move(alloc))
        {}
        trie_impl& operator=(trie_impl&&) = default;
    };

    trie_impl impl_;
};

} // namespace rmr::detail
