// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <rmr/detail/util.h>
#include <rmr/detail/trie_node_base.h>

#include <rmr/detail/trie.h>

namespace rmr::detail {

template <typename T, typename Char>
struct ternary_search_tree_node : trie_node_base<ternary_search_tree_node<T, Char>, T, 3> { Char c = 0; };

template <typename T, typename Char, typename OStream>
void write_dot_nodes(ternary_search_tree_node<T, Char>* node, OStream& os) {
    os << "  node [shape = " << (node->value == nullptr ? "circle" : "doublecircle") << "];";
    os << "  \"" << node << "\" [label = " << node->c << "];\n";

    if (node->children[0] != nullptr) write_dot_nodes(node->children[0], os);
    if (node->children[1] != nullptr) write_dot_nodes(node->children[1], os);
    if (node->children[2] != nullptr) write_dot_nodes(node->children[2], os);
}

template <typename T, typename Char, typename OStream>
void write_dot_impl(ternary_search_tree_node<T, Char>* node, OStream& os) {
    write_dot_nodes(node, os);

    if (node->children[0] != nullptr) {
        os << "  \"" << node << "\" -> \"" << node->children[0] << "\" [label = l];\n";
        write_dot_impl(node->children[0], os);
    }

    if (node->children[1] != nullptr) {
        os << "  \"" << node << "\" -> \"" << node->children[1] << "\" [label = m, style = dashed];\n";
        write_dot_impl(node->children[1], os);
    }

    if (node->children[2] != nullptr) {
        os << "  \"" << node << "\" -> \"" << node->children[2] << "\" [label = r];\n";
        write_dot_impl(node->children[2], os);
    }
}

template <typename Node>
struct ternary_search_tree_iterator_traits {
    using node_type = Node;
    static constexpr std::size_t radix = 3;
    template <typename _Node> using rebind = ternary_search_tree_iterator_traits<_Node>;

    static node_type skip(node_type) { return nullptr; }
    static node_type next(node_type) { return nullptr; }
    static node_type prev(node_type) { return nullptr; }
};

template <typename T, typename Compare, typename Key, typename Allocator>
class ternary_search_tree {
    static constexpr std::size_t R = 3;
    using alloc_traits = std::allocator_traits<Allocator>;
public:
    using key_type        = Key;
    using char_type       = typename key_type::value_type;
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using key_compare     = Compare;
    using allocator_type  = Allocator;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using pointer         = typename alloc_traits::pointer;
    using const_pointer   = typename alloc_traits::const_pointer;
private:
    using node_type             = ternary_search_tree_node<value_type, char_type>;
    using node_allocator_type   = typename alloc_traits::template rebind_alloc<node_type>;
    using node_alloc_traits     = typename alloc_traits::template rebind_traits<node_type>;
    using iterator_traits       = ternary_search_tree_iterator_traits<node_type*>;
    using const_iterator_traits = ternary_search_tree_iterator_traits<const node_type*>;
public:
    using iterator               = trie_iterator<iterator_traits>;
    using const_iterator         = trie_iterator<const_iterator_traits>;
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

    iterator reinsert(const_iterator pos, const key_type& key, const_pointer p)
    { return reinsert(remove_const(pos), key, const_cast<pointer>(p)); }
    iterator reinsert(iterator pos, const key_type& key, pointer p)
    { return insert_node(pos.node, key, p); }

    iterator find(const key_type& key)
    { return remove_const(const_cast<const ternary_search_tree&>(*this).find(key)); }
    const_iterator find(const key_type& key) const
    { return find_key(&impl_.root, key); }

    iterator erase(const_iterator pos) { return erase(remove_const(pos)); }
    iterator erase(iterator pos) {
        iterator next = std::next(pos);
        erase_node(pos.node);
        impl_.size -= 1;
        return next;
    }

    pointer extract(const_iterator pos) { return extract_value(remove_const(pos).node); }

    std::pair<const_iterator, const_iterator>
    prefixed_with(const key_type& key) const {
        const_iterator first = find_key_unsafe(&impl_.root, key, 0);
        if (first == end()) return { end(), end() };
        const_iterator last( const_iterator_traits::skip(first.node) );

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
    key_compare    key_comp()      const { return impl_; }

private:
    node_type* insert_node(const node_type* hint, const key_type& key, pointer v) {
        size_type rank = 0;
        const node_type* cur = hint;

        while (cur != &impl_.root) { if (cur->parent_index == 1) rank++; cur = cur->parent; }

        return insert_node(hint, rank, key, v);
    }
    node_type* insert_node(const node_type* _hint, size_type rank, const key_type& key, pointer v) {
        node_type* hint = const_cast<node_type*>(_hint);

        if (rank == 0 && impl_.size == 0) hint->c = key[0]; // fix root node character

        node_type* ret;
        insert_node(hint, hint->parent, hint->parent_index, key, rank, v, ret);
        return ret;
    }
    node_type* insert_node(
        node_type* root,
        node_type* parent,
        size_type parent_index,
        const key_type& key,
        size_type i,
        pointer v,
        node_type*& ret
    ) {
        char_type c = key[i];
        key_compare cmp = key_comp();
        if (root == nullptr) root = make_node(parent, parent_index, c);

        if (cmp(c, root->c))
            root->children[0] = insert_node(root->children[0], root, 0, key, i, v, ret);
        else if (cmp(root->c, c))
            root->children[2] = insert_node(root->children[2], root, 2, key, i, v, ret);
        else if (i < key.size() - 1)
            root->children[1] = insert_node(root->children[1], root, 1, key, i + 1, v, ret);
        else {
            if (root->value == nullptr) { root->value = v; ++impl_.size; }
            else destroy_and_deallocate(get_allocator(), v);
            ret = root;
        }
        return root;
    }

    const auto& get_node_allocator() const { return impl_; }
          auto& get_node_allocator()       { return impl_; }

    node_type* make_node(node_type* parent, size_type parent_index, char_type c) {
        auto& node_alloc = get_node_allocator();
        node_type* n = node_alloc_traits::allocate(node_alloc, 1);
        node_alloc_traits::construct(node_alloc, n);

        std::fill(std::begin(n->children), std::end(n->children), nullptr);
        n->parent = parent;
        n->parent_index = parent_index;
        n->value = nullptr;
        n->c = c;

        return n;
    }
 
    const node_type* find_key(const node_type* root, const key_type& key) const {
        const node_type* pos = find_key_unsafe(root, key, 0);
        if (pos->value == nullptr) return &impl_.base;
        return pos;
    }

    const node_type* find_key_unsafe(const node_type* root, const key_type& key, size_type i) const {
        if (root == nullptr) return &impl_.base;

        char_type c = key[i];
        key_compare cmp = key_comp();

        if      (cmp(c, root->c)   ) return find_key_unsafe(root->children[0], key, i    );
        else if (cmp(root->c, c)   ) return find_key_unsafe(root->children[2], key, i    );
        else if (i < key.size() - 1) return find_key_unsafe(root->children[1], key, i + 1);
        else                         return root;
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
        erase_node(node);
        impl_.size--;
        return v;
    }

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
