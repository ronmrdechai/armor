// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <rmr/trie_map.h>
#include <rmr/trie_set.h>

// using structs and not typedefs for CTest pretty types.
struct trie_map : rmr::trie_map<int, 127> { using rmr::trie_map<int, 127>::trie_map; };
struct trie_set : rmr::trie_set<127>      { using rmr::trie_set<127>::trie_set;      };

using assoc_map_types    = testing::Types<trie_map>;
using assoc_set_types    = testing::Types<trie_set>;
using assoc_common_types = testing::Types<trie_map, trie_set>;

namespace test {

template <typename T> typename T::value_type key_to_value(typename T::key_type) = delete;
template <> typename trie_map::value_type key_to_value<trie_map>(typename trie_map::key_type k) {
    return { k, typename trie_map::mapped_type{} };
}
template <> typename trie_set::value_type key_to_value<trie_set>(typename trie_set::key_type k) {
    return k;
}

template <typename T> typename T::key_type value_to_key(typename T::value_type) = delete;
template <> typename trie_map::key_type value_to_key<trie_map>(typename trie_map::value_type v) {
    return v.first;
}
template <> typename trie_set::key_type value_to_key<trie_set>(typename trie_set::value_type v) {
    return v;
}

template <typename Container, typename... Keys>
Container make_container(Keys... keys) {
    Container c;
    int _[]{ (c.insert(key_to_value<Container>(keys)), 0)... };
    (void)_;
    return c;
}

template <typename T> T less_trie = make_container<T>("aaa", "bbb", "ccc");
template <typename T> T greater_trie = make_container<T>("bbb", "ccc", "ddd");
template <typename T> T roman_trie = make_container<T>(
    "romane", "romanus", "romulus", "rubens", "ruber", "rubicon", "rubicundus"
);

template <typename T> void assert_empty(const T& t) {
    EXPECT_TRUE(t.empty());
    EXPECT_EQ(0u, t.size());
    EXPECT_EQ(0u, std::distance(t.begin(), t.end()));
}

namespace detail {
template <typename... Ts> struct make_void { typedef void type;};
template <typename... Ts> using void_t = typename make_void<Ts...>::type;
} // namespace detail

#define DEFINE_HAS(member)\
    template <typename, typename = detail::void_t<>> struct has_##member : std::false_type {};\
    template <typename T> struct has_##member<T, detail::void_t<typename T::member>> : std::true_type {}

DEFINE_HAS(iterator);
DEFINE_HAS(const_iterator);
DEFINE_HAS(reverse_iterator);
DEFINE_HAS(const_reverse_iterator);
DEFINE_HAS(node_type);
DEFINE_HAS(insert_return_type);
DEFINE_HAS(mapped_type);

#undef DEFINE_HAS

} // namespace test
