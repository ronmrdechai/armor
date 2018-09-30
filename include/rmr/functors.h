// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

namespace rmr {

template <typename T>
struct identity { T operator()(T v) const { return v; } };
template <typename T, T S>
struct count_from { T operator()(T v) const { return v - S; } };

template <typename T>
struct empty_string { bool operator()(const T&) const { return true; } };
template <>
struct empty_string<std::string> { bool operator()(const std::string& s) const { return s.size() == 0; } };

namespace detail {

template <typename T, T N, T V, T... Vs>
struct indexed_helper {
    T operator()(T v) {
        if (v == V) return N;
        return indexed_helper<T, N + 1, Vs...>{}(v);
    }
};

template <typename T, T N, T V>
struct indexed_helper<T, N, V> {
    T operator()(T v) {
        if (v == V) return N;
        return v;
    }
};

} // namespace detail

template <typename T, T... Vs>
struct indexed {
    T operator()(T v) const {
        return detail::indexed_helper<T, 0, Vs...>{}(v);
    }
};

} // namespace rmr
