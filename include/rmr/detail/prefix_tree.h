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

template <typename T, std::size_t R>
struct prefix_tree_node {
    using pointer       =       prefix_tree_node*;
    using const_pointer = const prefix_tree_node*;
    using value_type    = T;

    pointer 	children[R];
    std::size_t parent_index;
    pointer 	parent;
	T*			value;
};

template <typename T, std::size_t R>
std::size_t children_count(typename prefix_tree_node<T, R>::const_pointer n) {
	std::size_t count = 0;	
	for (auto& child : n->children) count += child != nullptr;
	return count;
}

template <typename T, std::size_t R>
void unlink(typename prefix_tree_node<T, R>::pointer n) {
	n->parent->children[n->parent_index] = nullptr;
}

template <typename T, std::size_t R, typename ValueAllocator>
void delete_node_value(typename prefix_tree_node<T, R>::pointer n, ValueAllocator& va) {
	if (n != nullptr && n->value = nullptr) {
		std::allocator_traits<ValueAllocator>::destroy(va, n->value);
		std::allocator_traits<ValueAllocator>::deallocate(va, n->value, 1);
		n->value = nullptr;
	}
}

template <typename T, std::size_t R, typename NodeAllocator, typename ValueAllocator>
void clear_node(typename prefix_tree_node<T, R>::pointer n, NodeAllocator& na, ValueAllocator& va) {
	delete_node_value(n, va);

	for (auto& child : n->children) {
		if (child != nullptr) {
			clear_node(child, na, va);	

			std::allocator_traits<NodeAllocator>::destroy(na, child);
			std::allocator_traits<NodeAllocator>::deallocate(na, child, 1);
			child = nullptr;
		}
	}
}

template <std::size_t R, typename NodeT>
struct prefix_tree_iterator {
    using value_type        = typename std::remove_pointer_t<NodeT>::value_type;
    using difference_type   = std::ptrdiff_t;
    using reference         = value_type&;
    using pointer           = value_type*;
    using iterator_category = std::bidirectional_iterator_tag;

    using node_type = NodeT;

    node_type node;

    prefix_tree_iterator(node_type n = nullptr) : node(n) {}
    prefix_tree_iterator(const prefix_tree_iterator&) = default;
    prefix_tree_iterator& operator=(const prefix_tree_iterator&) = default;

    template <typename = std::enable_if_t<std::is_const_v<node_type>>>
    prefix_tree_iterator(const prefix_tree_iterator<R, std::remove_const_t<node_type>>& other) :
        node(const_cast<const node_type*>(other.node))
    {}

    prefix_tree_iterator& operator++() {
        do {
            *this = next(*this);
        } while(this->node->value == nullptr && this->node->parent_index != R);
        return *this;
    }

    prefix_tree_iterator operator++(int) {
        prefix_tree_iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    reference operator*() const { return *(node->value); }
    pointer  operator->() const { return   node->value ; }

    bool operator==(const prefix_tree_iterator& other) const {
        return (node == other.node);
    }

    bool operator!=(const prefix_tree_iterator& other) const {
        return !(*this == other);
    }

    friend void swap(prefix_tree_iterator& lhs, prefix_tree_iterator& rhs) {
        std::swap(lhs.node, rhs.node);
    }
};

template <std::size_t R, typename NodeT>
std::pair<prefix_tree_iterator<R, NodeT>, bool>
step_down(prefix_tree_iterator<R, NodeT> it) {
    for (std::size_t pos = 0; pos < R; ++pos) {
        if (it.node->children[pos] != nullptr) {
            it.node = it.node->children[pos];
            return {it, true};
        }
    }
    return {it, false};
}

template <std::size_t R, typename NodeT>
std::pair<prefix_tree_iterator<R, NodeT>, bool>
step_right(prefix_tree_iterator<R, NodeT> it) {
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
std::pair<prefix_tree_iterator<R, NodeT>, bool>
step_up(prefix_tree_iterator<R, NodeT> it) {
    it.node = it.node->parent;
    return step_right(it);
}

template <std::size_t R, typename NodeT>
prefix_tree_iterator<R, NodeT> next(prefix_tree_iterator<R, NodeT> it) {
    bool stepped;

    std::tie(it, stepped) = step_down(it);
    if (stepped) return it;
    return skip(it);
}

template <std::size_t R, typename NodeT>
prefix_tree_iterator<R, NodeT> skip(prefix_tree_iterator<R, NodeT> it) {
    bool stepped;

    std::tie(it, stepped) = step_right(it);
    if (stepped) return it;

    while (it.node->parent_index != R) {
        std::tie(it, stepped) = step_up(it);
        if (stepped) return it;
    }
    return it;
}

template <std::size_t R, typename NodeT>
auto remove_const(prefix_tree_iterator<R, NodeT> it) {
    using non_const_node = std::add_pointer_t<std::remove_const_t<std::remove_pointer_t<NodeT>>>;
    return prefix_tree_iterator<R, non_const_node>(const_cast<non_const_node>(it.node));
}

template <typename T, std::size_t R, typename KeyMapper, typename Key, typename Allocator>
class prefix_tree {
    static_assert(
        std::is_invocable_r_v<std::size_t, KeyMapper, std::size_t>,
        "KeyMapper is not invocable on std::size_t or does not return std::size_t"
    );
    using alloc_traits        = std::allocator_traits<Allocator>;
    using node_type           = prefix_tree_node<T, R>;
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
    using iterator               = prefix_tree_iterator<R, node_type*>;
    using const_iterator         = prefix_tree_iterator<R, const node_type*>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    template <typename... Args>
    iterator emplace(iterator pos, const key_type& key, Args&&... args) {
        auto alloc = get_allocator();
        value_type* v = alloc_traits::allocate(alloc, 1);
        alloc_traits::construct(alloc, v, std::forward<Args>(args)...);
        return iterator(insert_node(pos.node, key, v));
    }

    iterator root() { return remove_const(croot()); }
    const_iterator root() const { return croot(); }
    const_iterator croot() const { return const_iterator(&impl_.root); }

    iterator begin() { return remove_const(cbegin()); }
    const_iterator begin() const { return cbegin(); }
    const_iterator cbegin() const { return ++const_iterator(&impl_.base); }

    iterator end() { return remove_const(cend()); }
    const_iterator end() const { return cend(); }
    const_iterator cend() const { return const_iterator(&impl_.base); }

    size_type size() const { return impl_.size; }
    bool empty() const { return size() == 0; }

    allocator_type get_allocator() const { return {get_node_allocator()}; }
    key_mapper     key_map()       const { return impl_; }

private:
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
            else { auto alloc = get_allocator(); alloc_traits::deallocate(alloc, v, 1); }
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

    struct prefix_tree_header {
        using node_type = prefix_tree_node<T, R>;

        node_type   base;
        node_type   root;
        std::size_t size;

        prefix_tree_header() { reset(); }
        prefix_tree_header(prefix_tree_header&& other) : prefix_tree_header() {
            for (std::size_t i = 0; i < R; ++i)
                root.children[i] = other.root.children[i];
            size = other.size;
            other.reset();
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

    struct prefix_tree_impl : prefix_tree_header, key_mapper, node_allocator_type {
        prefix_tree_impl(prefix_tree_impl&&) = default;
        prefix_tree_impl() : prefix_tree_header(), key_mapper(), node_allocator_type() {}
        prefix_tree_impl(prefix_tree_header&& header) :
            prefix_tree_header(std::move(header)),
            key_mapper(), node_allocator_type()
        {}
        prefix_tree_impl(prefix_tree_header&& header, key_mapper km) :
            prefix_tree_header(std::move(header)),
            key_mapper(std::move(km)), node_allocator_type()
        {}
        prefix_tree_impl(prefix_tree_header&& header, key_mapper km, node_allocator_type alloc) :
            prefix_tree_header(std::move(header)),
            key_mapper(std::move(km)),
            node_allocator_type(std::move(alloc))
        {}
    };

    prefix_tree_impl impl_;
};

} // namespace rmr::detail
