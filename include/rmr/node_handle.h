// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <rmr/detail/util.h>

namespace rmr {

namespace detail {
template <typename T, typename Trie> class map_adaptor;
template <typename T, typename Trie> class set_adaptor;
} // namespace detail

template <typename Allocator>
class node_handle_common {
    using alloc_traits = std::allocator_traits<Allocator>;
public:
    using allocator_type = Allocator;

    [[nodiscard]] bool empty() const noexcept { return ptr_ == nullptr; }
    explicit operator bool() const noexcept { return !empty(); }

    allocator_type get_allocator() const { return alloc_; }

protected:
    constexpr node_handle_common() noexcept : ptr_(nullptr), empty_() {}
    node_handle_common(typename alloc_traits::pointer ptr, allocator_type alloc) :
        ptr_(ptr), alloc_(std::move(alloc)) {}

    node_handle_common(node_handle_common&& other) noexcept : ptr_(other.ptr_), alloc_(other.alloc_)
    { other.ptr_ = nullptr; other.alloc_.~allocator_type(); }

    ~node_handle_common() { destroy(); alloc_.~allocator_type(); }

    node_handle_common& operator=(node_handle_common&& other) noexcept {
        destroy();
        ptr_ = other.ptr_;
        if (alloc_traits::propagate_on_container_move_assignment::value) alloc_ = std::move(other.alloc_);
        other.ptr_ = nullptr;
        other.alloc_.~allocator_type();
        return *this;
    }

    void swap_(node_handle_common& other) noexcept {
        if (alloc_traits::propagate_on_container_swap::value || empty() || other.empty()) {
            if (empty() && !other.empty())
            { alloc_ = std::move(other.alloc_); other.alloc_.~allocator_type(); }
            else if (!empty && other.empty())
            { other.alloc_ = std::move(alloc_); alloc_.~allocator_type(); }
            else alloc_.swap(other.alloc_);
        }
        using std::swap;
        swap(ptr_, other.ptr_); 
    }

    typename alloc_traits::pointer ptr_;
private:
    union {
        allocator_type alloc_;
        struct{} empty_;
    };

    void destroy() noexcept {
        if (empty()) return;
        allocator_type alloc(alloc_);
        detail::destroy_and_deallocate(alloc, ptr_);
    }
};

template <typename Key, typename Value, typename Allocator>
class node_handle : public node_handle_common<Allocator> {
    using alloc_traits = std::allocator_traits<Allocator>;
public:
    constexpr node_handle() noexcept = default;
    node_handle(node_handle&&) noexcept = default;
    ~node_handle() = default;

    node_handle& operator=(node_handle&&) noexcept = default;

    using key_type = Key;
    using mapped_type = typename Value::second_type;

    key_type& key() const noexcept { return *pkey_; }
    mapped_type& mapped() const noexcept { return *pmapped_; }

    void swap(node_handle&& other) noexcept(
        alloc_traits::propagate_on_container_swap::value || alloc_traits::is_always_equal::value
    ) {
        swap_(other);
        using std::swap;
        swap(pkey_, other.pkey_);
        swap(pmapped_, other.pmapped_);
    }

private:
    node_handle(typename alloc_traits::pointer ptr, Allocator alloc) :
        node_handle_common<Allocator>(ptr, std::move(alloc))
    {
        if (ptr) {
            auto& key = const_cast<Key&>(ptr->first);
            pkey_ = pointer_to(key);
            pmapped_ = pointer_to(ptr->second);
        } else {
            pkey_ = nullptr;
            pmapped_ = nullptr;
        }
    }

    using pointer_traits = std::pointer_traits<typename alloc_traits::pointer>;
    template <typename T>
    using pointer_traits_of = std::pointer_traits<
        typename pointer_traits::template rebind<std::remove_reference_t<T>>
    >;

    typename pointer_traits_of<Key>::pointer pkey_;
    typename pointer_traits_of<typename Value::second_type>::pointer pmapped_;

    template <typename T>
    static typename pointer_traits_of<T>::pointer pointer_to(T& o)
    { return pointer_traits_of<T>::pointer_to(o); }

    template <typename T, typename Trie> friend class detail::map_adaptor;
    template <typename T, typename Trie> friend class detail::set_adaptor;
};

template <typename Value, typename Allocator>
class node_handle<Value, Value, Allocator> : public node_handle_common<Allocator> {
    using alloc_traits = std::allocator_traits<Allocator>;
public:
    constexpr node_handle() noexcept = default;
    node_handle(node_handle&&) noexcept = default;
    ~node_handle() = default;

    node_handle& operator=(node_handle&&) noexcept = default;

    using value_type = Value;

    value_type& value() const noexcept { return *ptr_; }

    void swap(node_handle&& other) noexcept(
        alloc_traits::propagate_on_container_swap::value || alloc_traits::is_always_equal::value
    ) { swap_(other); }

private:
    node_handle(typename alloc_traits::pointer ptr, Allocator alloc) :
        node_handle_common<Allocator>(ptr, std::move(alloc)) {}

    value_type& key() const noexcept { return value(); }

    template <typename T, typename Trie> friend class detail::set_adaptor;
};

template <typename Iterator, typename NodeType>
struct node_insert_return {
    Iterator position = Iterator();
    bool inserted = false;
    NodeType node;
};

} // namespace rmr
