#pragma once

namespace moat {

template <typename T>
struct identity { T operator()(T v) const { return v; } };
template <typename T, T S>
struct count_from { T operator()(T v) const { return v - S; } };

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

} // namespace moat
