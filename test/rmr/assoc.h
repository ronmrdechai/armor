// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <fstream>

#include <rmr/meta.h>
#include <rmr/trie_map.h>
#include <rmr/trie_set.h>
#include <rmr/tst_map.h>
#include <rmr/tst_set.h>

// using structs and not typedefs for CTest pretty types.
struct trie_map : rmr::trie_map<int, 127> { using rmr::trie_map<int, 127>::trie_map; };
struct trie_set : rmr::trie_set<127>      { using rmr::trie_set<127>::trie_set;      };

struct tst_map : rmr::tst_map<int> { using rmr::tst_map<int>::tst_map; };
struct tst_set : rmr::tst_set<>    { using rmr::tst_set<>::tst_set;      };

using assoc_map_types    = testing::Types<trie_map, tst_map>;
using assoc_set_types    = testing::Types<trie_set, tst_set>;
using assoc_common_types = testing::Types<trie_map, trie_set, tst_map, tst_set>;
using trie_common_types  = testing::Types<trie_map, trie_set>;
using tst_common_types   = testing::Types<tst_map, tst_set>;

namespace test {

#define DEFINE_HAS(member)\
    template <typename T> using has_##member = typename T::member

DEFINE_HAS(iterator);
DEFINE_HAS(const_iterator);
DEFINE_HAS(reverse_iterator);
DEFINE_HAS(const_reverse_iterator);
DEFINE_HAS(node_type);
DEFINE_HAS(insert_return_type);
DEFINE_HAS(mapped_type);
DEFINE_HAS(key_compare);
DEFINE_HAS(key_mapper);

template <typename T> typename T::value_type key_to_value(typename T::key_type) = delete;

template <> typename trie_map::value_type key_to_value<trie_map>(typename trie_map::key_type k)
{ return { k, typename trie_map::mapped_type{} }; }
template <> typename trie_set::value_type key_to_value<trie_set>(typename trie_set::key_type k)
{ return k; }
template <> typename tst_map::value_type key_to_value<tst_map>(typename tst_map::key_type k)
{ return { k, typename tst_map::mapped_type{} }; }
template <> typename tst_set::value_type key_to_value<tst_set>(typename tst_set::key_type k)
{ return k; }

template <typename T> typename T::key_type value_to_key(typename T::value_type) = delete;

template <> typename trie_map::key_type value_to_key<trie_map>(typename trie_map::value_type v)
{ return v.first; }
template <> typename trie_set::key_type value_to_key<trie_set>(typename trie_set::value_type v)
{ return v; }
template <> typename tst_map::key_type value_to_key<tst_map>(typename tst_map::value_type v)
{ return v.first; }
template <> typename tst_set::key_type value_to_key<tst_set>(typename tst_set::value_type v)
{ return v; }

template <typename T> typename T::key_type nh_to_key(typename T::node_type&&) = delete;

template <> typename trie_map::key_type nh_to_key<trie_map>(typename trie_map::node_type&& nh)
{ return nh.key(); }
template <> typename trie_set::key_type nh_to_key<trie_set>(typename trie_set::node_type&& nh)
{ return nh.value(); }
template <> typename tst_map::key_type nh_to_key<tst_map>(typename tst_map::node_type&& nh)
{ return nh.key(); }
template <> typename tst_set::key_type nh_to_key<tst_set>(typename tst_set::node_type&& nh)
{ return nh.value(); }

template <typename T, typename... Keys>
T make_container(Keys... keys) { return T( { key_to_value<T>(keys)... } ); }

template <typename T> T less_trie    = make_container<T>("aaa", "bbb", "ccc");
template <typename T> T greater_trie = make_container<T>("bbb", "ccc", "ddd");
template <typename T> T roman_trie   = make_container<T>(
    "romane", "romanus", "romulus", "rubens", "ruber", "rubicon", "rubicundus"
);

template <typename T> void assert_empty(const T& t) {
    EXPECT_TRUE(t.empty());
    EXPECT_EQ(0u, t.size());
    EXPECT_EQ(0u, std::distance(t.begin(), t.end()));
}

#undef DEFINE_HAS

template <typename T, std::size_t N = 0>
struct counting_allocator : std::allocator<T> {
    using std::allocator<T>::allocator;

    T* allocate(std::size_t n, const void* hint)
    { bytes_allocated += sizeof(T) * n; return std::allocator<T>::allocate(n, hint); }
    T* allocate(std::size_t n)
    { bytes_allocated += sizeof(T) * n; return std::allocator<T>::allocate(n); }
    void deallocate(T* p, std::size_t n)
    { bytes_allocated -= sizeof(T) * n; std::allocator<T>::deallocate(p, n); }

    static std::size_t bytes_allocated;
};
template <typename T, std::size_t N>
std::size_t counting_allocator<T, N>::bytes_allocated = 0;

template <typename, typename> struct replace_alloc;

template <typename Alloc>
struct replace_alloc<Alloc, trie_map>
{ using type = rmr::trie_map<int, 127, rmr::identity<std::size_t>, std::string, Alloc>; };
template <typename Alloc>
struct replace_alloc<Alloc, trie_set>
{ using type = rmr::trie_set<127, rmr::identity<std::size_t>, std::string, Alloc>; };

template <typename Alloc>
struct replace_alloc<Alloc, tst_map>
{ using type = rmr::tst_map<int, std::less<char>, std::string, Alloc>; };
template <typename Alloc>
struct replace_alloc<Alloc, tst_set>
{ using type = rmr::tst_set<std::less<char>, std::string, Alloc>; };

template <typename T>
struct assoc_test {
    bool has_iterator               = rmr::is_detected_v<test::has_iterator, T>;
    bool has_const_iterator         = rmr::is_detected_v<test::has_const_iterator, T>;
    bool has_reverse_iterator       = rmr::is_detected_v<test::has_reverse_iterator, T>;
    bool has_const_reverse_iterator = rmr::is_detected_v<test::has_const_reverse_iterator, T>;
    bool has_node_type              = rmr::is_detected_v<test::has_node_type, T>;
    bool has_insert_return_type     = rmr::is_detected_v<test::has_insert_return_type, T>;
    bool has_key_compare            = rmr::is_detected_v<test::has_key_compare, T>;
    bool has_mapped_type            = rmr::is_detected_v<test::has_mapped_type, T>;
    bool has_key_mapper             = rmr::is_detected_v<test::has_key_mapper, T>;

    template <typename Alloc>
    using replace_alloc = typename test::replace_alloc<Alloc, T>::type;

    T less_trie    = test::less_trie<T>;
    T greater_trie = test::greater_trie<T>;
    T roman_trie   = test::roman_trie<T>;

    static typename T::value_type
    key_to_value(const typename T::key_type& k) { return test::key_to_value<T>(k); }
    static typename T::key_type
    value_to_key(const typename T::value_type& v) { return test::value_to_key<T>(v); }
    static typename T::key_type
    nh_to_key(typename T::node_type&& nh) { return test::nh_to_key<T>(std::move(nh)); }
    template <typename... Keys>
    static T make_container(Keys&&... keys) { return test::make_container<T>(std::forward<Keys>(keys)...); }
    static void assert_empty(const T& t) { test::assert_empty<T>(t); }

    static void write_dot(const T& container) {
        std::string path = ::testing::UnitTest::GetInstance()->current_test_info()->name();
        path += ".dot";
        rmr::detail::write_dot(container.__croot(), std::ofstream(path));
    }
};

} // namespace test
