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
struct ternary_search_tree_node : trie_node_base<ternary_search_tree_node<T, Char>, T, 3> {
    Char c = 0;

    auto& left()         { return this->children[0]; }
    auto  left()   const { return this->children[0]; }
    auto& middle()       { return this->children[1]; }
    auto  middle() const { return this->children[1]; }
    auto& right()        { return this->children[2]; }
    auto  right()  const { return this->children[2]; }
};

template <typename T, typename Char>
void unlink(ternary_search_tree_node<T, Char>* n) {
    if (n->parent->left()   == n) n->parent->left()   = nullptr;
    if (n->parent->middle() == n) n->parent->middle() = nullptr;
    if (n->parent->right()  == n) n->parent->right()  = nullptr;
}

template <typename T, typename Char, typename OStream>
void write_dot_nodes(const ternary_search_tree_node<T, Char>* node, OStream& os) {
    os << "  node [shape = " << (node->value == nullptr ? "circle" : "doublecircle") << "];";
    os << "  \"" << node << "\" [label = " << node->c << "];\n";

    if (node->left()   != nullptr) write_dot_nodes(node->left(),   os);
    if (node->middle() != nullptr) write_dot_nodes(node->middle(), os);
    if (node->right()  != nullptr) write_dot_nodes(node->right(),  os);
}

template <typename T, typename Char, typename OStream>
void write_dot_impl(const ternary_search_tree_node<T, Char>* node, OStream& os) {
    write_dot_nodes(node, os);

    if (node->left() != nullptr) {
        os << "  \"" << node << "\" -> \"" << node->left() << "\" [label = l];\n";
        write_dot_impl(node->left(), os);
    }

    if (node->middle() != nullptr) {
        os << "  \"" << node << "\" -> \"" << node->middle() << "\" [label = m, style = dashed];\n";
        write_dot_impl(node->middle(), os);
    }

    if (node->right() != nullptr) {
        os << "  \"" << node << "\" -> \"" << node->right() << "\" [label = r];\n";
        write_dot_impl(node->right(), os);
    }
}

template <typename Node>
struct ternary_search_tree_iterator_traits : trie_iterator_traits_base<3, Node> {
    template <typename _Node>
    using rebind = ternary_search_tree_iterator_traits<_Node>;

    static bool is_left_child(Node n)   { return n == n->parent->left(); }
    static bool is_middle_child(Node n) { return n == n->parent->middle(); }
    static bool is_right_child(Node n)  { return n == n->parent->right(); }

    static Node tree_min(Node n) {
        if (n->left() != nullptr) return tree_min(n->left());
        return n;
    }

    static Node tree_max(Node n) {
        if (n->right()  != nullptr) return tree_max(n->right());
        if (n->middle() != nullptr) return tree_max(n->middle());
        return n;
    }

    static Node skip_forward(Node n) {
        while (n->parent != nullptr && !is_left_child(n) && children_count(n->parent) == 1) n = n->parent;

        if (is_left_child(n)) return n->parent;
        if (is_middle_child(n) && n->parent->right() != nullptr) return n->parent->right();
        return skip_forward(n->parent); // is right child
    }

    static Node skip_backward(Node n) {
        while (n->parent != nullptr && !is_right_child(n) && children_count(n->parent) == 1) n = n->parent;

        if (is_right_child(n)) return n->parent;
        if (is_middle_child(n) && n->parent->left() != nullptr) return tree_max(n->parent->left());
        return skip_backward(n->parent); // is left child
    }

    static Node next(Node n) {
        if (n->middle() != nullptr) return tree_min(n->middle());
        if (n->right()  != nullptr) return tree_min(n->right());
        return skip_forward(n);
    }

    static Node prev(Node n) {
        if (n->left()   != nullptr) return tree_max(n->left());
        if (n->middle() != nullptr) return tree_max(n->middle());
        return skip_backward(n);
    }
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

    ternary_search_tree(const ternary_search_tree& other) : ternary_search_tree(
        other.key_comp(),
        alloc_traits::select_on_container_copy_construction(other.get_allocator())
    ) {
        copy_nodes(&other.impl_.root, &impl_.root, nullptr);
        impl_.size = other.impl_.size;
    }
    ternary_search_tree(const ternary_search_tree& other, allocator_type alloc) :
        ternary_search_tree(other.key_comp(), std::move(alloc))
    {
        copy_nodes(&other.impl_.root, &impl_.root, nullptr);
        impl_.size = other.impl_.size;
    }

    ternary_search_tree(ternary_search_tree&& other) :
        ternary_search_tree(std::move(other.key_comp()), std::move(other.get_allocator()))
    {
        static_cast<ternary_search_tree_header&>(impl_) =
            std::move(static_cast<ternary_search_tree_header&>(other.impl_));
    }
    ternary_search_tree(ternary_search_tree&& other, allocator_type alloc) :
        ternary_search_tree(std::move(other.key_comp()), std::move(alloc))
    {
        if (alloc != other.get_allocator()) {
            auto other_alloc = other.get_allocator();
            move_nodes(other_alloc, &other.impl_.root, &impl_.root, nullptr);
            other.clear();
        } else {
            static_cast<ternary_search_tree_header&>(impl_) =
                std::move(static_cast<ternary_search_tree_header&>(other.impl_));
        }
    }
    ~ternary_search_tree() { clear(); }

    ternary_search_tree& operator=(const ternary_search_tree& other) {
        clear();
        impl_.size = other.impl_.size;
        static_cast<key_compare&>(impl_) = other.key_comp();
        if (alloc_traits::propagate_on_container_copy_assignment::value) {
            static_cast<node_allocator_type&>(impl_) = other.get_node_allocator();
        }
        copy_nodes(&other.impl_.root, &impl_.root, nullptr);
        return *this;
    }

    ternary_search_tree& operator=(ternary_search_tree&& other) noexcept(
        alloc_traits::is_always_equal::value && std::is_nothrow_move_assignable<key_compare>::value
    ) {
        clear();
        static_cast<key_compare&>(impl_) = other.key_comp();
        if (alloc_traits::propagate_on_container_move_assignment::value)
            static_cast<node_allocator_type&>(impl_) = other.get_node_allocator();

        auto other_alloc = other.get_allocator();
        if (!alloc_traits::propagate_on_container_move_assignment::value &&
                get_allocator() != other.get_allocator()) {
            move_nodes(other_alloc, &other.impl_.root, &impl_.root, nullptr);
            impl_.size = other.impl_.size;
        } else {
            static_cast<ternary_search_tree_header&>(impl_) =
                std::move(static_cast<ternary_search_tree_header&>(other.impl_));
        }

        other.clear();
        return *this;
    }

    void swap(ternary_search_tree& other) noexcept(
        alloc_traits::is_always_equal::value && std::is_nothrow_swappable<key_compare>::value
    ) {
        if (alloc_traits::propagate_on_container_swap::value) {
            node_allocator_type& this_alloc = impl_;
            static_cast<node_allocator_type&>(impl_) = other.get_node_allocator();
            other.get_node_allocator() = this_alloc;
        }
        key_compare this_key_comp = key_comp();
        static_cast<key_compare&>(impl_) = other.key_comp();
        static_cast<key_compare&>(other.impl_) = this_key_comp;
        for (size_type i = 0; i < R; ++i) {
            std::swap(impl_.root.children[i], other.impl_.root.children[i]);
            if (impl_.root.children[i] != nullptr)
                impl_.root.children[i]->parent = &impl_.root;
            if (other.impl_.root.children[i] != nullptr)
                other.impl_.root.children[i]->parent = &other.impl_.root;
        }
        std::swap(impl_.size,   other.impl_.size);
        std::swap(impl_.root.c, other.impl_.root.c);
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

    iterator longest_match(const key_type& key)
    { return remove_const(const_cast<const ternary_search_tree&>(*this).longest_match(key)); }
    const_iterator longest_match(const key_type& key) const
    { return longest_match(&impl_.root, key); }

    std::pair<iterator, iterator> prefixed_with(const key_type& key) {
        auto p = const_cast<const ternary_search_tree&>(*this).prefixed_with(key);
        return { remove_const(p.first), remove_const(p.second) };
    }
    std::pair<const_iterator, const_iterator>
    prefixed_with(const key_type& key) const {
        const_iterator first = find_key_unsafe(&impl_.root, key, 0);
        if (first == end()) return { end(), end() };
        const_iterator last;
        if (first.node->right() != nullptr) last = first.node->right();
        else                                last = const_iterator_traits::skip_forward(first.node);

        if (                first.node->value == nullptr) ++first;
        if (last != end() && last.node->value == nullptr) ++last;
        return {first, last};
    }

    iterator root() noexcept { return remove_const(croot()); }
    const_iterator root() const noexcept { return croot(); }
    const_iterator croot() const noexcept { return &impl_.root; }

    iterator begin() noexcept { return remove_const(cbegin()); }
    const_iterator begin() const noexcept { return cbegin(); }
    const_iterator cbegin() const noexcept {
        const node_type* n = const_iterator_traits::tree_min(&impl_.root);
        if (n->value != nullptr) return n;
        return ++const_iterator(n);
    }

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
    node_type* copy_nodes(const node_type* src, node_type* dst, node_type* parent) {
        if (dst == nullptr) dst = make_node(parent, src->c);
        dst->c = src->c;

        if (src->value != nullptr) dst->value = make_value(get_allocator(), *src->value);
        for (size_type i = 0; i < R; ++i) {
            if (src->children[i] != nullptr)
                dst->children[i] = copy_nodes(src->children[i], dst->children[i], dst);
        }
        return dst;
    }

    node_type* move_nodes(allocator_type& alloc, node_type* src, node_type* dst, node_type* parent) {
        if (dst == nullptr) dst = make_node(parent, src->c);
        dst->c = src->c;

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

        while (cur != &impl_.root) { if (cur == cur->parent->middle()) rank++; cur = cur->parent; }

        return insert_node(hint, rank, key, v);
    }
    node_type* insert_node(const node_type* _hint, size_type rank, const key_type& key, pointer v) {
        node_type* hint = const_cast<node_type*>(_hint);

        if (rank == 0 && impl_.size == 0) hint->c = key[0]; // fix root node character

        node_type* ret;
        insert_node(hint, hint->parent, key, rank, v, ret);
        return ret;
    }
    node_type* insert_node(
        node_type* root,
        node_type* parent,
        const key_type& key,
        size_type i,
        pointer v,
        node_type*& ret
    ) {
        char_type c = key[i];
        key_compare cmp = key_comp();
        if (root == nullptr) root = make_node(parent, c);

        if (cmp(c, root->c))
            root->left()   = insert_node(root->left(),   root, key, i,     v, ret);
        else if (cmp(root->c, c))
            root->right()  = insert_node(root->right(),  root, key, i,     v, ret);
        else if (i < key.size() - 1)
            root->middle() = insert_node(root->middle(), root, key, i + 1, v, ret);
        else {
            if (root->value == nullptr) { root->value = v; ++impl_.size; }
            else destroy_and_deallocate(get_allocator(), v);
            ret = root;
        }
        return root;
    }

    const auto& get_node_allocator() const { return impl_; }
          auto& get_node_allocator()       { return impl_; }

    node_type* make_node(node_type* parent, char_type c) {
        auto& node_alloc = get_node_allocator();
        node_type* n = node_alloc_traits::allocate(node_alloc, 1);
        node_alloc_traits::construct(node_alloc, n);

        std::fill(std::begin(n->children), std::end(n->children), nullptr);
        n->parent = parent;
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

        if      (cmp(c, root->c)   ) return find_key_unsafe(root->left(),   key, i    );
        else if (cmp(root->c, c)   ) return find_key_unsafe(root->right(),  key, i    );
        else if (i < key.size() - 1) return find_key_unsafe(root->middle(), key, i + 1);
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

    const node_type* longest_match(const node_type* root, const key_type& key) const {
        auto pos = longest_match_candidate(root, root->parent, key, 0);
        while (pos->value == nullptr && pos->parent != nullptr) pos = pos->parent;
        return pos;
    }
    const node_type* longest_match_candidate(
        const node_type* node, const node_type* prev, const key_type& key, size_type i
    ) const {
        if (node == nullptr) return prev;

        char_type c = key[i];
        key_compare cmp = key_comp();
        prev = node;

        if      (cmp(c, node->c)   ) return longest_match_candidate(node->left(),   prev, key, i    );
        else if (cmp(node->c, c)   ) return longest_match_candidate(node->right(),  prev, key, i    );
        else if (i < key.size() - 1) return longest_match_candidate(node->middle(), prev, key, i + 1);
        else                         return node;
    }

    struct ternary_search_tree_header {
        node_type base;
        node_type root;
        size_type size;

        ternary_search_tree_header() { reset(); }
        ternary_search_tree_header& operator=(ternary_search_tree_header&& other) {
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
            base.left() = &root;
            base.parent = nullptr;
            base.value = nullptr;

            std::fill(std::begin(root.children), std::end(root.children), nullptr);
            root.parent = &base;
            root.value = nullptr;

            size = 0;
        }
    };

    struct ternary_search_tree_impl : ternary_search_tree_header, key_compare, node_allocator_type {
        ternary_search_tree_impl() = default;
        ternary_search_tree_impl(node_allocator_type alloc) :
            ternary_search_tree_header(), key_compare(), node_allocator_type(std::move(alloc))
        {}
        ternary_search_tree_impl(key_compare kc, node_allocator_type alloc) :
            ternary_search_tree_header(), key_compare(std::move(kc)), node_allocator_type(std::move(alloc))
        {}
        ternary_search_tree_impl& operator=(ternary_search_tree_impl&&) = default;
    };

    ternary_search_tree_impl impl_;
};

} // namespace rmr::detail
