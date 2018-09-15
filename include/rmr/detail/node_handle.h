// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <rmr/detail/util.h>

namespace rmr {

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

    node_handle_common(node_handle_common&& other) noexcept : ptr_(other.ptr_), alloc_(other.alloc_)
    { other.ptr_ = nullptr; other.alloc_.~allocator_type(); }

    ~node_handle_common() { destroy(); alloc_.~allocator_type(); }

    node_handle_common& operator=(node_handle_common&& other) {
        destroy();
        ptr_ = other.ptr_;
        if (alloc_traits::propagate_on_container_move_assignment::value) alloc_ = std::move(other.alloc_);
        other.ptr_ = nullptr;
        other.alloc_.~allocator_type();
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
}

template <typename Key, typename Value, typename Allocator>
class node_handle : public node_handle_common<Allocator> {
public:
    constexpr node_handle() noexcept = default;
    node_handle(node_handle&&) noexcept = default;
    ~node_handle() = default;

    node_handle& operator=(node_handle&&) noexcept = default;

    using key_type = Key;
    using mapped_type = typename Value::second_type;

    key_type& key() const noexcept { return *pkey_; }
    mapped_type& mapped() const noexcept { return *pmapped_; }

    void swap(node_handle&& other) noexcept {
        swap_(other);
        using std::swap;
        swap(pkey_, other.pkey_);
        swap(pmapped_, other.pmapped_);
    }

private:
    node_handle(typename alloc_traits::pointer ptr, allocator_type alloc) :
        node_handle_common(ptr, alloc)
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
    using pointer_traits_of = typename pointer_traits::template rebind<std::remove_reference_t<T>>;

    typename pointer_traits_of<Key>::pointer pkey_;
    typename pointer_traits_of<typename Value::second_type>::pointer pmapped_;

    template <typename T>
    static typename pointer_traits_of<T>::pointer pointer_to(T& o)
    { return pointer_traits_of<T>::pointer_to(o); }

    template <typename T, typename Trie> friend class map_adaptor;
};

template <typename Value, typename Allocator>
class node_handle<Value, Value, Allocator> : public node_handle_common<Allocator> { /* TODO */ };

} // namespace rmr
