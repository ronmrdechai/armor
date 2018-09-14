// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

namespace rmr::detail {

template <typename Iterator, typename NodeType> struct insert_return {
    Iterator position;
    bool     inserted;
    NodeType node;
};

} // namespace rmr::detail
