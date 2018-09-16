// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <rmr/detail/map_adaptor.h>
#include <rmr/detail/trie.h>
#include <rmr/functors.h>

namespace rmr {

template <
    typename T,
    std::size_t R,
    typename KeyMapper = identity<std::size_t>,
    typename Key = std::string,
    typename Allocator = std::allocator<std::pair<const Key, T>>
>
class trie_map : public detail::map_adaptor<
    T, detail::trie<typename Allocator::value_type, R, KeyMapper, Key, Allocator>
> {
    static_assert(
        std::is_invocable_r_v<std::size_t, KeyMapper, std::size_t>,
        "KeyMapper is not invocable on std::size_t or does not return std::size_t"
    );
    using base_type = detail::map_adaptor<
        T, detail::trie<typename Allocator::value_type, R, KeyMapper, Key, Allocator>
    >;
public:
    using base_type::base_type;

    static constexpr std::size_t radix() { return R; }
};

} // namespace rmr
