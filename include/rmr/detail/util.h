#pragma once

#include <memory>

namespace rmr::detail {

template <typename Alloc, typename Pointer>
inline void destroy_and_deallocate(Alloc& alloc, Pointer&& p) {
    std::allocator_traits<Alloc>::destroy(alloc, std::forward<Pointer>(p));
    std::allocator_traits<Alloc>::deallocate(alloc, std::forward<Pointer>(p), 1);
}

} // namespace rmr::detail
