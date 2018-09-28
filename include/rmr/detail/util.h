#pragma once

#include <memory>

namespace rmr::detail {

template <typename Alloc, typename Pointer>
inline void destroy_and_deallocate(Alloc&& alloc, Pointer&& p) {
    using alloc_type = std::remove_reference_t<Alloc>;
    std::allocator_traits<alloc_type>::destroy(alloc, std::forward<Pointer>(p));
    std::allocator_traits<alloc_type>::deallocate(alloc, std::forward<Pointer>(p), 1);
}

template <typename Alloc, typename... Args>
inline typename Alloc::pointer make_value(Alloc&& alloc, Args&&... args) {
    using alloc_type = std::remove_reference_t<Alloc>;
    typename alloc_type::pointer v = std::allocator_traits<alloc_type>::allocate(alloc, 1);
    std::allocator_traits<alloc_type>::construct(alloc, v, std::forward<Args>(args)...);
    return v;
}

} // namespace rmr::detail
