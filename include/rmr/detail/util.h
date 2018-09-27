#pragma once

#include <memory>

namespace rmr::detail {

template <typename Alloc, typename Pointer>
inline void destroy_and_deallocate(Alloc& alloc, Pointer&& p) {
    std::allocator_traits<Alloc>::destroy(alloc, std::forward<Pointer>(p));
    std::allocator_traits<Alloc>::deallocate(alloc, std::forward<Pointer>(p), 1);
}

template <typename Alloc, typename... Args>
inline typename Alloc::pointer make_value(Alloc&& alloc, Args&&... args) {
    typename Alloc::pointer v = std::allocator_traits<Alloc>::allocate(alloc, 1);
    std::allocator_traits<Alloc>::construct(alloc, v, std::forward<Args>(args)...);
    return v;
}

} // namespace rmr::detail
