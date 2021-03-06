// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <functional>

#include <rmr/detail/set_adaptor.h>
#include <rmr/detail/ternary_search_tree.h>

namespace rmr {

template <
    typename Compare = std::less<char>,
    typename Key = std::string,
    typename Allocator = std::allocator<Key>
>
class tst_set : public detail::set_adaptor<
    detail::ternary_search_tree<typename Allocator::value_type, Compare, Key, Allocator>
> {
    static_assert(
        std::is_invocable_r_v<bool, Compare, typename Key::value_type, typename Key::value_type>,
        "Compare is not invocable with Key::value_type or does not return bool"
    );
    using base_type = detail::set_adaptor<
        detail::ternary_search_tree<typename Allocator::value_type, Compare, Key, Allocator>
    >;
public:
    using base_type::base_type;
    using key_compare = Compare;

    Compare key_comp() const { return this->trie_.key_comp(); }
};

} // namespace rmr
