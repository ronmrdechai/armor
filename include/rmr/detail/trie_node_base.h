// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <iterator>
#include <type_traits>

#include <rmr/detail/util.h>

namespace rmr::detail {

template <typename Derived, typename T, std::size_t R>
struct trie_node_base {
    using value_type = T;

    Derived*    children[R];
    std::size_t parent_index;
    Derived*    parent;
	T*          value;
};

template <typename Node>
std::size_t children_count(const Node* n) {
	std::size_t count = 0;	
	for (auto& child : n->children) count += child != nullptr;
	return count;
}

template <typename Node>
void unlink(Node* n) {
	n->parent->children[n->parent_index] = nullptr;
}

template <typename Node, typename ValueAllocator>
void delete_node_value(Node* n, ValueAllocator& va) {
	if (n != nullptr && n->value != nullptr) {
        destroy_and_deallocate(va, n->value);
		n->value = nullptr;
	}
}

template <typename Node, typename NodeAllocator, typename ValueAllocator>
void clear_node(Node* n, NodeAllocator& na, ValueAllocator& va) {
	delete_node_value(n, va);

	for (auto& child : n->children) {
		if (child != nullptr) {
			clear_node(child, na, va);	
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

    using non_const_node_type = std::add_pointer_t<std::remove_const_t<std::remove_pointer_t<NodeT>>>;
    static constexpr bool is_const_iterator = std::is_const_v<std::remove_pointer_t<NodeT>>;

    NodeT node;

    trie_iterator(NodeT n = nullptr) : node(n) {}
    trie_iterator(const trie_iterator&) = default;

    template <typename = std::enable_if_t<is_const_iterator>>
    trie_iterator(const trie_iterator<R, non_const_node_type>& other) :
        node(const_cast<NodeT>(other.node))
    {}

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

} // namespace rmr::detail
