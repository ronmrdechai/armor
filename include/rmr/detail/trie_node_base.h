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

template <typename TrieIterator, typename OStream>
void write_dot(TrieIterator it, OStream& os) {
    os << "digraph trie {\n";
    write_dot_impl(it.node, os);
    os << "}\n";
}

template <typename Traits>
struct trie_iterator {
    using node_type = typename Traits::node_type;

    using value_type        = typename std::remove_pointer_t<node_type>::value_type;
    using difference_type   = std::ptrdiff_t;
    using reference         = value_type&;
    using pointer           = value_type*;
    using iterator_category = std::bidirectional_iterator_tag;

    using non_const_node_type = std::add_pointer_t<std::remove_const_t<std::remove_pointer_t<node_type>>>;
    using non_const_traits_type = typename Traits::template rebind<non_const_node_type>;

    node_type node;

    trie_iterator(node_type n = nullptr) : node(n) {}
    trie_iterator(const trie_iterator&) = default;

    template <typename = std::enable_if_t<Traits::is_const>>
    trie_iterator(const trie_iterator<non_const_traits_type>& other) :
        node(const_cast<node_type>(other.node))
    {}

    trie_iterator& operator=(const trie_iterator&) = default;

    trie_iterator& operator++() {
        do {
            node = Traits::next(node);
        } while(node->value == nullptr && node->parent_index != Traits::radix);
        return *this;
    }

    trie_iterator operator++(int) {
        trie_iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    trie_iterator& operator--() {
        do {
            node = Traits::prev(node);
        } while(node->value == nullptr && node->parent_index != Traits::radix);
        return *this;
    }

    trie_iterator operator--(int) {
        trie_iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    reference operator*() const { return *(node->value); }
    pointer  operator->() const { return   node->value ; }

    template <typename _Traits>
    bool operator==(const trie_iterator<_Traits>& other) const {
        return (node == other.node);
    }

    template <typename _Traits>
    bool operator!=(const trie_iterator<_Traits>& other) const {
        return !(*this == other);
    }

    friend void swap(trie_iterator& lhs, trie_iterator& rhs) {
        std::swap(lhs.node, rhs.node);
    }
};

template <typename Traits>
auto remove_const(trie_iterator<Traits> it) {
    using non_const_node_type = typename trie_iterator<Traits>::non_const_node_type; 
    using non_const_traits_type = typename trie_iterator<Traits>::non_const_traits_type; 
    return trie_iterator<non_const_traits_type>(const_cast<non_const_node_type>(it.node));
}

template <std::size_t R, typename Node>
struct trie_iterator_traits_base {
    using node_type = Node;

    static constexpr std::size_t radix = R;
    static constexpr bool is_const = std::is_const_v<std::remove_pointer_t<node_type>>;

    static std::pair<node_type, bool> step_down_forward(node_type n) {
        for (std::size_t pos = 0; pos < R; ++pos) {
            if (n->children[pos] != nullptr) {
                n = n->children[pos];
                return {n, true};
            }
        }
        return {n, false};
    }

    static std::pair<node_type, bool> step_down_backward(node_type n) {
        for (std::size_t pos_ = R; pos_ > 0; --pos_) {
            std::size_t pos = pos_ - 1;

            if (n->children[pos] != nullptr) {
                n = n->children[pos];
                return {n, true};
            }
        }
        return {n, false};
    }

    static std::pair<node_type, bool> step_right(node_type n) {
        if (n->parent_index == R) return {n, false};

        for (std::size_t pos = n->parent_index + 1; pos < R; ++pos) {
            Node parent = n->parent;
            if (parent->children[pos] != nullptr) {
                n = parent->children[pos];
                return {n, true};
            }
        }
        return {n, false};
    }

    static std::pair<node_type, bool> step_left(node_type n) {
        if (n->parent_index == 0) return {n, false};

        for (std::size_t pos_ = n->parent_index; pos_ > 0; --pos_) {
            std::size_t pos = pos_ - 1;

            node_type parent = n->parent;
            if (parent->children[pos] != nullptr) {
                n = parent->children[pos];

                bool stepped;
                do std::tie(n, stepped) = step_down_backward(n); while (stepped);

                return {n, true};
            }
        }
        return {n, false};
    }

    static std::pair<node_type, bool> step_up_forward(node_type n) {
        n = n->parent;
        return step_right(n);
    }

    static std::pair<node_type, bool> step_up_backward(node_type n) {
        n = n->parent;
        if (n->value != nullptr) return {n, true};
        else                     return step_left(n);
    }

};

} // namespace rmr::detail
