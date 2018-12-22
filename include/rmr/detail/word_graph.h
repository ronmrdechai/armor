// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <algorithm>

#include <rmr/detail/util.h>
#include <rmr/detail/trie_node_base.h>

namespace rmr::detail {

template <typename T>
class array_list {
public:
    using size_type = std::size_t;
    using iterator = T*;
    using const_iterator = const T*;
    using reference = T&;
    using const_reference = const T&;

    reference operator[](size_type n) { return data_[n]; }
    const_reference operator[](size_type n) const { return data_[n]; }

    iterator begin() { return &data_[0]; }
    const_iterator begin() const { return &data_[0]; }
    const_iterator cbegin() const { return &data_[0]; }
    iterator end() { return &data_[size_]; }
    const_iterator end() const { return &data_[size_]; }
    const_iterator cend() const { return &data_[size_]; }

    size_type size() const { return size_; }
    size_type capacity() const { return capacity_; }


    template <typename Allocator, typename... Args>
    void emplace_at(Allocator& alloc, size_type index, Args&&... args) {
        if (size_ == capacity_) reallocate_shifted(alloc, index, capacity_ * 2);
        else if (index < size_) std::move(&data_[index], end(), &data_[index + 1]);

        std::allocator_traits<Allocator>::construct(alloc, &data_[index], std::forward<Args>(args)...);
        size_++;
    }

    template <typename Allocator>
    void remove_at(Allocator& alloc, size_type index) {
        std::allocator_traits<Allocator>::destroy(alloc, &data_[index]);
        if (index < size_) std::move_backward(&data_[index + 1], end(), &data_[index]);
        size_--;
        if (size_ <= capacity_ / 4) reallocate(alloc, capacity_ / 2);
    }

    template <typename Allocator, typename... Args>
    void emplace_back(Allocator& alloc, Args&&... args) {
        if (size_ == capacity_) reallocate(alloc, capacity_ * 2);
        std::allocator_traits<Allocator>::construct(alloc, &data_[size_], std::forward<Args>(args)...);
        size_++;
    }
    template <typename Allocator>
    void remove_back(Allocator& alloc) {
        std::allocator_traits<Allocator>::destroy(alloc, &data_[size_]);
        size_--;
        if (size_ <= capacity_ / 4) reallocate(alloc, capacity_ / 2);
    }

    template <typename Allocator>
    void clear(Allocator& alloc) {
        deallocate(alloc);
        size_ = 0;
        capacity_ = 0;
    }

private:
    template <typename Allocator>
    void reallocate(Allocator& alloc, size_type n) {
        if (n == 0) n = 1;

        T* new_data = std::allocator_traits<Allocator>::allocate(alloc, n);
        std::move(begin(), end(), new_data);
        deallocate(alloc);
        data_ = new_data;
        capacity_ = n;
    }

    template <typename Allocator>
    void reallocate_shifted(Allocator& alloc, size_type index, size_type n) {
        if (n == 0) n = 1;

        T* new_data = std::allocator_traits<Allocator>::allocate(alloc, n);
        std::move(begin(), &data_[index], new_data);
        std::move(&data_[index], end(), new_data + index + 1);
        deallocate(alloc);
        data_ = new_data;
        capacity_ = n;
    }

    template <typename Allocator>
    void deallocate(Allocator& alloc) {
        for (auto it = begin(); it != end(); ++it)
            std::allocator_traits<Allocator>::destroy(alloc, it);
        std::allocator_traits<Allocator>::deallocate(alloc, data_, capacity_);
        data_ = nullptr;
    }

    T*        data_     = nullptr;
    size_type size_     = 0;
    size_type capacity_ = 0;
};

template <std::size_t R>
struct word_graph_node : trie_node_base<word_graph_node<R>, void, R> { bool accepting; };

template <typename T, std::size_t R, typename KeyMapper, typename Key, typename Allocator>
class word_graph {
    using alloc_traits        = std::allocator_traits<Allocator>;
    using node_type           = word_graph_node<R>;
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
private:
    using storage_type           = array_list<value_type>;
    using const_key_iterator     = typename key_type::const_iterator;
public:
    using iterator               = typename storage_type::iterator;
    using const_iterator         = typename storage_type::const_iterator;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    word_graph() = default;
    explicit word_graph(allocator_type alloc) : impl_(node_allocator_type(std::move(alloc))) {}
    explicit word_graph(key_mapper km, allocator_type alloc) :
        impl_(std::move(km), node_allocator_type(std::move(alloc)))
    {}
    ~word_graph() { clear(); }

    node_type* root() noexcept { return &impl_.root; }
    const node_type* root() const noexcept { return cbegin(); }
    const node_type* croot() const noexcept { return &impl_.root; }

    iterator begin() noexcept { return remove_const(cbegin()); }
    const_iterator begin() const noexcept { return cbegin(); }
    const_iterator cbegin() const noexcept { return data_.begin(); }

    iterator end() noexcept { return remove_const(cend()); }
    const_iterator end() const noexcept { return cend(); }
    const_iterator cend() const noexcept { return data_.end(); }

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
        data_.clear(value_alloc);

        clear_node(&impl_.root, node_alloc, value_alloc);
        impl_.size = 0;
    }

    allocator_type get_allocator() const { return get_node_allocator(); }
    key_mapper     key_map()       const { return impl_; }

/* private: */
    size_type insert_word_sorted();
    size_type insert_word_minimal();

    node_type* insert_word(node_type* root, const_key_iterator first, const_key_iterator last) {
        if (root == nullptr) root = make_node();
        if (first == last) root->accepting = true;
        else {
            size_type i = key_map()(*first);
            auto& child = root->children[i];
            child = insert_word(child, ++first, last);
        }
        return root;
    }

    const_key_iterator
    common_prefix(node_type* root, const_key_iterator first, const_key_iterator last) const;

    const auto& get_node_allocator() const { return impl_; }
          auto& get_node_allocator()       { return impl_; }

    node_type* make_node() {
        auto& node_alloc = get_node_allocator();
        node_type* n = node_alloc_traits::allocate(node_alloc, 1);
        node_alloc_traits::construct(node_alloc, n);

        std::fill(std::begin(n->children), std::end(n->children), nullptr);
        n->value = nullptr;
        n->accepting = false;

        return n;
    }

    struct word_graph_header {
        node_type root;
        size_type size;

        struct { const_key_iterator first, last; } last_string;

        word_graph_header() { reset(); }
        word_graph_header& operator=(word_graph_header&& other) {
            (void)other; // TODO
        }
        void reset() {
            std::fill(std::begin(root.children), std::end(root.children), nullptr);
            root.value = nullptr;
            root.accepting = false;
            size = 0;
        }
    };

    struct word_graph_impl : word_graph_header, key_mapper, node_allocator_type {
        word_graph_impl() = default;
        word_graph_impl(node_allocator_type alloc) :
            word_graph_header(), key_mapper(), node_allocator_type(std::move(alloc))
        {}
        word_graph_impl(key_mapper km, node_allocator_type alloc) :
            word_graph_header(), key_mapper(std::move(km)), node_allocator_type(std::move(alloc))
        {}
        word_graph_impl& operator=(word_graph_impl&&) = default;
    };

    word_graph_impl impl_;
    storage_type    data_;
};

} // namespace rmr::detail
