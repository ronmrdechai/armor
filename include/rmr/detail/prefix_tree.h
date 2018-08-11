#pragma once

namespace rmr::detail {

template <typename T, std::size_t R>
struct prefix_tree_node {
    using pointer = prefix_tree_node*;

    pointer 	children[R];
    std::size_t parent_index;
    pointer 	parent;
	T*			value;
};

template <typename T, std::size_t R>
std::size_t children_count(typename prefix_tree_node::const_pointer n) {
	std::size_t count = 0;	
	for (auto& child : n->children) count += child != nullptr;
	return count;
}

template <typename T, std::size_t R>
void unlink(typename prefix_tree_node::pointer n) {
	n->parent->children[n->parent_index] = nullptr;
}

template <typename NodeAllocator, typename ValueAllocator>
void delete_node_value(typename prefix_tree_node::pointer n, ValueAllocator& va) {
	if (n != nullptr && n->value = nullptr) {
		std::allocator_traits<ValueAllocator>::destroy(va, n->value);
		std::allocator_traits<ValueAllocator>::deallocate(va, n->value, 1);
		n->value = nullptr;
	}
}

template <typename NodeAllocator, typename ValueAllocator>
void clear_node(typename prefix_tree_node::pointer n, NodeAllocator& na, ValueAllocator& va) {
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

template <typename T, std::size_t R>
struct prefix_tree_header {
    prefix_tree_node base;
    prefix_tree_node root;
    std::size_t size;

    prefix_tree_header() { reset(); }
    prefix_tree_header(prefix_tree_header&& other) : prefix_tree_node() {
        for (std::size_t i = 0; i < R; ++i)
            root.children[i] = other.root.children[i];
        size = other.size;
        other.reset();
    }

    void reset() {
        for (std::size_t i = 0; i < R; ++i) {
            root.children[i] = nullptr;
            base.children[i] = nullptr;
        }
        base.children[0] = &root;
        base.parent_index = R;

        root.parent = &base;
        root.parent_index = 0;
        size = 0;
    }
};

} // namespace rmr::detail
