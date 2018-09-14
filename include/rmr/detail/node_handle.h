// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

namespace rmr::detail {

template <typename...> class node_handle;

template <typename Value, typename Allocator>
class node_handle<Value, Allocator> {
};

template <typename Key, typename Value, typename Allocator>
class node_handle<Key, Value, Allocator> {
};

} // namespace rmr::detail
