// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <iterator>
#include <type_traits>

namespace rmr::detail {

template <typename Alloc, typename Pointer>
inline void destroy_and_deallocate(Alloc& alloc, Pointer&& p) {
    std::allocator_traits<Alloc>::destroy(alloc, std::forward<Pointer>(p));
    std::allocator_traits<Alloc>::deallocate(alloc, std::forward<Pointer>(p), 1);
}

template <typename T, std::size_t R>
struct trie_node {
    using value_type = T;

    trie_node*  children[R];
    std::size_t parent_index;
    trie_node*  parent;
	T*          value;
};

template <typename T, std::size_t R>
std::size_t children_count(const trie_node<T, R>* n) {
	std::size_t count = 0;	
	for (auto& child : n->children) count += child != nullptr;
	return count;
}

template <typename T, std::size_t R>
void unlink(trie_node<T, R>* n) {
	n->parent->children[n->parent_index] = nullptr;
}

template <typename T, std::size_t R, typename ValueAllocator>
void delete_node_value(trie_node<T, R>* n, ValueAllocator& va) {
	if (n != nullptr && n->value != nullptr) {
        destroy_and_deallocate(va, n->value);
		n->value = nullptr;
	}
}

template <typename T, std::size_t R, typename NodeAllocator, typename ValueAllocator>
void clear_node(trie_node<T, R>* n, NodeAllocator& na, ValueAllocator& va) {
	delete_node_value(n, va);

	for (auto& child : n->children) {
		if (child != nullptr) {
			clear_node(child, na, va);	
            destroy_and_deallocate(va, n->value);
			child = nullptr;
		}
	}
}

template <std::size_t R, typename NodeT>
struct trie_iterator {
    using value_type        = typename std::remove_pointer_t<NodeT>::value_type;
    using difference_type   = std::ptrdiff_t;
    using reference         = value_type&;
    using pointer           = value_type*;
    using iterator_category = std::bidirectional_iterator_tag;

    NodeT node;

    trie_iterator(NodeT n = nullptr) : node(n) {}
    trie_iterator(const trie_iterator&) = default;
    trie_iterator& operator=(const trie_iterator&) = default;

    trie_iterator& operator++() {
        do {
            *this = next(*this);
        } while(node->value == nullptr && node->parent_index != R);
        return *this;
    }

    trie_iterator operator++(int) {
        trie_iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    trie_iterator& operator--() {
        do {
            *this = prev(*this);
        } while(node->value == nullptr && node->parent_index != R);
        return *this;
    }

    trie_iterator operator--(int) {
        trie_iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    reference operator*() const { return *(node->value); }
    pointer  operator->() const { return   node->value ; }

    template <typename _NodeT>
    bool operator==(const trie_iterator<R, _NodeT>& other) const {
        return (node == other.node);
    }

    template <typename _NodeT>
    bool operator!=(const trie_iterator<R, _NodeT>& other) const {
        return !(*this == other);
    }

    friend void swap(trie_iterator& lhs, trie_iterator& rhs) {
        std::swap(lhs.node, rhs.node);
    }
};

template <std::size_t R, typename NodeT>
std::pair<trie_iterator<R, NodeT>, bool>
step_down_forward(trie_iterator<R, NodeT> it) {
    for (std::size_t pos = 0; pos < R; ++pos) {
        if (it.node->children[pos] != nullptr) {
            it.node = it.node->children[pos];
            return {it, true};
        }
    }
    return {it, false};
}

template <std::size_t R, typename NodeT>
std::pair<trie_iterator<R, NodeT>, bool>
step_down_backward(trie_iterator<R, NodeT> it) {
    for (std::size_t pos_ = R; pos_ > 0; --pos_) {
        std::size_t pos = pos_ - 1;

        if (it.node->children[pos] != nullptr) {
            it.node = it.node->children[pos];
            return {it, true};
        }
    }
    return {it, false};
}

template <std::size_t R, typename NodeT>
std::pair<trie_iterator<R, NodeT>, bool>
step_right(trie_iterator<R, NodeT> it) {
    if (it.node->parent_index == R) return {it, false};

    for (std::size_t pos = it.node->parent_index + 1; pos < R; ++pos) {
        NodeT parent = it.node->parent;
        if (parent->children[pos] != nullptr) {
            it.node = parent->children[pos];
            return {it, true};
        }
    }
    return {it, false};
}

template <std::size_t R, typename NodeT>
std::pair<trie_iterator<R, NodeT>, bool>
step_left(trie_iterator<R, NodeT> it) {
    if (it.node->parent_index == 0) return {it, false};

    for (std::size_t pos_ = it.node->parent_index; pos_ > 0; --pos_) {
        std::size_t pos = pos_ - 1;

        NodeT parent = it.node->parent;
        if (parent->children[pos] != nullptr) {
            it.node = parent->children[pos];

            bool stepped;
            do std::tie(it, stepped) = step_down_backward(it); while (stepped);

            return {it, true};
        }
    }
    return {it, false};
}

template <std::size_t R, typename NodeT>
std::pair<trie_iterator<R, NodeT>, bool>
step_up_forward(trie_iterator<R, NodeT> it) {
    it.node = it.node->parent;
    return step_right(it);
}

template <std::size_t R, typename NodeT>
std::pair<trie_iterator<R, NodeT>, bool>
step_up_backward(trie_iterator<R, NodeT> it) {
    it.node = it.node->parent;
    if (it.node->value != nullptr) return {it, true};
    else                           return step_left(it);
}

template <std::size_t R, typename NodeT>
trie_iterator<R, NodeT> skip(trie_iterator<R, NodeT> it) {
    bool stepped;

    std::tie(it, stepped) = step_right(it);
    if (stepped) return it;

    while (it.node->parent_index != R) {
        std::tie(it, stepped) = step_up_forward(it);
        if (stepped) return it;
    }
    return it;
}

template <std::size_t R, typename NodeT>
trie_iterator<R, NodeT> next(trie_iterator<R, NodeT> it) {
    bool stepped;

    std::tie(it, stepped) = step_down_forward(it);
    if (stepped) return it;
    return skip(it);
}

template <std::size_t R, typename NodeT>
trie_iterator<R, NodeT> prev(trie_iterator<R, NodeT> it) {
    bool stepped;

    std::tie(it, stepped) = step_down_backward(it);
    if (stepped) return it;

    std::tie(it, stepped) = step_left(it);
    if (stepped) return it;

    while (it.node->parent_index != R) {
        std::tie(it, stepped) = step_up_backward(it);
        if (stepped) return it;
    }
    return it;
}

template <std::size_t R, typename NodeT>
auto remove_const(trie_iterator<R, NodeT> it) {
    using non_const_node = std::add_pointer_t<std::remove_const_t<std::remove_pointer_t<NodeT>>>;
    return trie_iterator<R, non_const_node>(const_cast<non_const_node>(it.node));
}

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
    iterator emplace(const_iterator pos, const key_type& key, Args&&... args) {
        return emplace(remove_const(pos), key, std::forward<Args>(args)...);
    }
    template <typename... Args>
    iterator emplace(iterator pos, const key_type& key, Args&&... args) {
        return insert_node(pos.node, key, make_value(std::forward<Args>(args)...));
    }

    const_iterator find(const key_type& key) const {
        return find_key(&impl_.root, key.begin(), key.end());
    }

    iterator erase(const_iterator pos) { return erase(remove_const(pos)); }
    iterator erase(iterator pos) {
        iterator next = std::next(pos);
        erase_node(pos.node);
        impl_.size -= 1;
        return next;
    }

    const_iterator longest_match(const key_type& key) const {
        return longest_match(&impl_.root, key.begin(), key.end());
    }

    std::pair<const_iterator, const_iterator>
    prefixed_with(const key_type& key) const {
        const_iterator first = find_key_unsafe(&impl_.root, key.begin(), key.end());
        if (first == end()) return { end(), end() };
        auto last = skip(first);

        if ( last.node->value == nullptr) ++last;
        if (first.node->value == nullptr) ++first;

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

        if (src->value != nullptr) dst->value = make_value(*src->value);
        for (size_type i = 0; i < R; ++i) {
            if (src->children[i] != nullptr)
                dst->children[i] = copy_nodes(src->children[i], dst->children[i], dst);
        }
        return dst;
    }

    node_type* move_nodes(allocator_type& alloc, node_type* src, node_type* dst, node_type* parent) {
        if (dst == nullptr) dst = make_node(parent, src->parent_index);

        if (src->value != nullptr) {
            dst->value = make_value(std::move(*src->value));
            destroy_and_deallocate(alloc, src->value);
            src->value = nullptr; 
        }
        for (size_type i = 0; i < R; ++i) {
            if (src->children[i] != nullptr)
                dst->children[i] = move_nodes(alloc, src->children[i], dst->children[i], dst);
        }
        return dst;
    }

    node_type* insert_node(const node_type* hint, const key_type& key, value_type* v) {
        size_type rank = 0;
        const node_type* cur = hint;
        while (cur != &impl_.root) { ++rank; cur = cur->parent; }
        return insert_node(hint, rank, key, v);
    }
    node_type* insert_node(const node_type* _hint, size_type rank, const key_type& key, value_type* v) {
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
        value_type* v,
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

    template <typename... Args>
    value_type* make_value(Args&&... args) {
        auto alloc = get_allocator();
        value_type* v = alloc_traits::allocate(alloc, 1);
        alloc_traits::construct(alloc, v, std::forward<Args>(args)...);
        return v;
    }

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
        using node_type = trie_node<T, R>;

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
