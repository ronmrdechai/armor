// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#include <gtest/gtest.h>

#include <experimental/memory_resource>
#include <rmr/trie_map.h>

using trie_map = rmr::trie_map<int, 127>;

namespace fixtures {

const trie_map roman_trie{
    {"romane", 1},
    {"romanus", 1},
    {"romulus", 1},
    {"rubens", 1},
    {"ruber", 1},
    {"rubicon", 1},
    {"rubicundus", 1}
};

} // namespace fixtures

TEST(trie_map, write_and_read) {
    trie_map t;
    t["foo"] = 1;
    t["bar"] = 2;
    EXPECT_EQ(1, t["foo"]);
    EXPECT_EQ(2, t["bar"]);
}

TEST(trie_map, safe_access_read) {
    trie_map t;
    t["foo"] = 1;
    t["bar"] = 2;
    EXPECT_EQ(1, t.at("foo"));
    EXPECT_EQ(2, t.at("bar"));
}

TEST(trie_map, safe_access_write) {
    trie_map t;
    t["foo"] = 1;
    t.at("foo") = 2;
    EXPECT_EQ(2, t["foo"]);
}

TEST(trie_map, safe_access_throws) {
    trie_map t;

    ASSERT_THROW(t.at("foo"), std::out_of_range);
    ASSERT_THROW(t.at("foo") = 1, std::out_of_range);
}

TEST(trie_map, default_is_empty) {
    trie_map t;
    EXPECT_TRUE(t.empty());
}

TEST(trie_map, not_empty_after_write) {
    trie_map t;
    t["foo"] = 1;
    EXPECT_FALSE(t.empty());
}

TEST(trie_map, empty_after_clear) {
    trie_map t;
    t["foo"] = 1;
    t.clear();
    EXPECT_TRUE(t.empty());
}

TEST(trie_map, default_size_is_zero) {
    trie_map t;
    EXPECT_EQ(0u, t.size());
}

TEST(trie_map, size_increase_after_write) {
    trie_map t;
    t["foo"] = 1;
    EXPECT_EQ(1u, t.size());
    t["bar"] = 1;
    EXPECT_EQ(2u, t.size());
}

TEST(trie_map, default_count_is_zero) {
    trie_map t;
    EXPECT_EQ(0u, t.count("foo"));
}

TEST(trie_map, count_increase_after_write) {
    trie_map t;
    t["foo"] = 1;
    EXPECT_EQ(1u, t.count("foo"));
}

TEST(trie_map, read_iteration) {
    std::vector<std::string> strings{ "bar", "baz", "foo" };
    trie_map t;

    for (std::string& s : strings) t[s] = 42;

    std::size_t i = 0;
    for (auto& [key, value] : t) {
        EXPECT_EQ(strings[i++], key);
        EXPECT_EQ(42, value);
    }
}

TEST(trie_map, write_iteration) {
    std::vector<std::string> strings{ "bar", "baz", "foo" };
    trie_map t;

    for (std::string& s : strings) t[s] = 42;
    for (auto& [_, value] : t) value = 0;
    for (std::string& s : strings) EXPECT_EQ(0, t[s]);
}

TEST(trie_map, find_existant) {
    trie_map t;
    t["foo"] = 1;
    auto it = t.find("foo");
    EXPECT_EQ(t.begin(), it);
}

TEST(trie_map, find_non_existant) {
    trie_map t;
    EXPECT_EQ(t.end(), t.find("foo"));
}

TEST(trie_map, try_emplace_and_access) {
    trie_map t;
    t.try_emplace("foo", 1);
    EXPECT_EQ(1, t["foo"]);
}

TEST(trie_map, try_emplace_twice) {
    trie_map t;

    bool emplaced;
    typename trie_map::iterator it1, it2;
    std::tie(it1, emplaced) = t.try_emplace("foo", 1);
    EXPECT_TRUE(emplaced);

    std::tie(it2, emplaced) = t.try_emplace("foo", 2);
    EXPECT_FALSE(emplaced);
    EXPECT_EQ(it1, it2);
    EXPECT_EQ(1, t["foo"]);
}

TEST(trie_map, try_emplace_hint) {
    trie_map t;

    auto [hint, _] = t.try_emplace("foo", 1);
    t.try_emplace(hint, "foobar", 2);

    EXPECT_EQ(1, t["foo"]);
    EXPECT_EQ(2, t["foobar"]);
}

TEST(trie_map, try_emplace_hint_exists) {
    trie_map t{ {"foobar", 3} };

    auto [hint, _] = t.insert( {"foo", 1} );

    EXPECT_EQ(t.find("foobar"), t.try_emplace(hint, "foobar", 2));

    EXPECT_EQ(1, t["foo"]);
    EXPECT_EQ(3, t["foobar"]);
}

TEST(trie_map, try_emplace_hint_wrong_hint) {
    trie_map t;

    auto [hint, _] = t.insert( {"bar", 1} );
    t.try_emplace(hint, "foobar", 2);

    EXPECT_EQ(1, t["bar"]);
    EXPECT_EQ(2, t["barbar"]);
}

TEST(trie_map, insert_and_access) {
    trie_map t;
    typename trie_map::value_type v{"foo", 1};
    t.insert(v);
    EXPECT_EQ(1, t["foo"]);
}

TEST(trie_map, insert_move_and_access) {
    trie_map t;
    typename trie_map::value_type v{"foo", 1};
    t.insert(std::move(v));
    EXPECT_EQ(1, t["foo"]);
}

TEST(trie_map, insert_twice) {
    trie_map t;

    bool inserted;
    typename trie_map::iterator it1, it2;
    std::tie(it1, inserted) = t.insert( {"foo", 1} );
    EXPECT_TRUE(inserted);

    std::tie(it2, inserted) = t.insert( {"foo", 2} );
    EXPECT_FALSE(inserted);
    EXPECT_EQ(it1, it2);
    EXPECT_EQ(1, t["foo"]);
}

TEST(trie_map, insert_prefix) {
    trie_map t{ {"foobar", 2} };

    auto [it, inserted] = t.insert( {"foo", 1} );
    EXPECT_TRUE(inserted);
    EXPECT_EQ(1, t["foo"]);
    EXPECT_EQ(2, t["foobar"]);
}

TEST(trie_map, insert_range) {
    trie_map t;
    std::vector<typename trie_map::value_type> v{
        {"bar", 1}, {"baz", 2}, {"foo", 3}
    };
    t.insert(v.begin(), v.end());

    std::size_t i = 0;
    for (auto& [key, value] : t) {
        EXPECT_EQ(v[i].first, key);
        EXPECT_EQ(v[i].second, value);
        ++i;
    }
}

TEST(trie_map, insert_list) {
    trie_map t;
    std::vector<typename trie_map::value_type> v{
        {"bar", 1}, {"baz", 2}, {"foo", 3}
    };
    t.insert({ {"bar", 1}, {"baz", 2}, {"foo", 3} });

    std::size_t i = 0;
    for (auto& [key, value] : t) {
        EXPECT_EQ(v[i].first, key);
        EXPECT_EQ(v[i].second, value);
        ++i;
    }
}

TEST(trie_map, insert_hint) {
    trie_map t;

    auto [hint, _] = t.insert( {"foo", 1} );
    t.insert(hint, {"foobar", 2});

    EXPECT_EQ(1, t["foo"]);
    EXPECT_EQ(2, t["foobar"]);
}

TEST(trie_map, insert_hint_handle) {
    trie_map t{ {"foobar", 3} };

    auto [hint, _] = t.insert( {"foo", 1} );
    auto nh = t.extract("foobar");
    t.insert(hint, std::move(nh));

    EXPECT_EQ(1, t["foo"]);
    EXPECT_EQ(3, t["foobar"]);
}

TEST(trie_map, insert_hint_exists) {
    trie_map t{ {"foobar", 3} };

    auto [hint, _] = t.insert( {"foo", 1} );

    EXPECT_EQ(t.find("foobar"), t.insert(hint, {"foobar", 2} ));

    EXPECT_EQ(1, t["foo"]);
    EXPECT_EQ(3, t["foobar"]);
}

TEST(trie_map, insert_hint_wrong_hint) {
    trie_map t;

    auto [hint, _] = t.insert( {"bar", 1} );
    t.insert(hint, {"foobar", 2} );

    EXPECT_EQ(1, t["bar"]);
    EXPECT_EQ(2, t["barbar"]);
}

TEST(trie_map, insert_or_assign_and_access) {
    trie_map t;
    t.insert_or_assign( {"foo", 1} );
    EXPECT_EQ(1, t["foo"]);
}

TEST(trie_map, insert_or_assign_twice) {
    trie_map t;

    bool inserted;
    typename trie_map::iterator it1, it2;
    std::tie(it1, inserted) = t.insert_or_assign( {"foo", 1} );
    EXPECT_TRUE(inserted);

    std::tie(it2, inserted) = t.insert_or_assign( {"foo", 2} );
    EXPECT_FALSE(inserted);
    EXPECT_EQ(it1, it2);
    EXPECT_EQ(2, t["foo"]);
}

TEST(trie_map, insert_or_assign_hint) {}
TEST(trie_map, insert_or_assign_hint_exists) {}
TEST(trie_map, insert_or_assign_hint_wrong_hint) {}

TEST(trie_map, copy_constructor) {
    trie_map t;
    t["foo"] = 1;
    t["bar"] = 2;

    trie_map s(t);
    EXPECT_EQ(t, s);
}

TEST(trie_map, change_original_after_copy) {
    trie_map t;
    t["foo"] = 1;
    t["bar"] = 2;

    trie_map s(t);

    t["foo"] = 3;
    EXPECT_NE(t, s);
}

TEST(trie_map, change_copy_after_copy) {
    trie_map t;
    t["foo"] = 1;
    t["bar"] = 2;

    trie_map s(t);

    s["foo"] = 3;
    EXPECT_NE(t, s);
}

TEST(trie_map, iterator_constructor) {
    std::vector<std::pair<std::string, int>> v{
        { "bar", 1 }, { "baz", 2 }, { "foo", 3 }
    };

    trie_map t(v.begin(), v.end());

    for (auto& [key, value] : v) {
        EXPECT_EQ(value, t[key]);
    }
}

TEST(trie_map, initializer_list_constructor) {
    std::vector<std::pair<std::string, int>> v{
        { "bar", 1 }, { "baz", 2 }, { "foo", 3 }
    };

    trie_map t{ {"bar", 1}, {"baz", 2}, {"foo", 3} };

    for (auto& [key, value] : v) {
        EXPECT_EQ(value, t[key]);
    }
}

TEST(trie_map, swap) {
    trie_map t{ {"foo", 1} };
    trie_map s{ {"bar", 2} };

    swap(t, s);
    EXPECT_EQ(2, t["bar"]);
    EXPECT_EQ(1, s["foo"]);
}

TEST(trie_map, swap_and_modify) {
    trie_map t{ {"foo", 1} };
    trie_map s{ {"bar", 2} };

    swap(t, s);
    t["foo"] = 3;

    EXPECT_EQ(2, t["bar"]);
    EXPECT_EQ(1, s["foo"]);
}

TEST(trie_map, erase_not_empty) {
    trie_map t{ {"foo", 1}, {"bar", 1}, {"baz", 1} };

    t.erase("foo");

    ASSERT_THROW(t.at("foo") = 1, std::out_of_range);
    EXPECT_EQ(1u, t.count("bar"));
    EXPECT_EQ(1u, t.count("baz"));

    t.erase("bar");
    ASSERT_THROW(t.at("bar") = 1, std::out_of_range);
    EXPECT_EQ(1u, t.count("baz"));
}

TEST(trie_map, erase_empty) {
    trie_map t{ {"foo", 1} };

    t.erase("foo");

    ASSERT_THROW(t.at("foo") = 1, std::out_of_range);
    // make sure trie_map still works
    t["bar"] = 1;
    EXPECT_EQ(1u, t.count("bar"));
}

TEST(trie_map, erase_prefix) {
    trie_map t{ {"foo", 1}, {"foobar", 2}, {"bar", 3} };
    t.erase("foo");

    ASSERT_THROW(t.at("foo") = 1, std::out_of_range);
    EXPECT_EQ(2, t.at("foobar"));
    EXPECT_EQ(3, t.at("bar"));
}

TEST(trie_map, erase_suffix) {
    trie_map t{ {"foo", 1}, {"foobar", 2}, {"bar", 3} };
    t.erase("foobar");

    ASSERT_THROW(t.at("foobar") = 1, std::out_of_range);
    EXPECT_EQ(1, t.at("foo"));
    EXPECT_EQ(3, t.at("bar"));
}

TEST(trie_map, erase_const_iterator) {
    trie_map t{ {"foo", 1}, {"bar", 1}, {"baz", 1} };

    typename trie_map::const_iterator it = const_cast<const trie_map&>(t).find("foo");
    t.erase(it);

    ASSERT_THROW(t.at("foo") = 1, std::out_of_range);
    EXPECT_EQ(1u, t.count("bar"));
    EXPECT_EQ(1u, t.count("baz"));
}

TEST(trie_map, erase_size_drop) {
    trie_map t{ {"foo", 1}, {"bar", 1}, {"baz", 1} };
    std::size_t size = t.size();
    t.erase("foo");
    EXPECT_EQ(size - 1, t.size());

    t.erase("foo");
    EXPECT_EQ(size - 1, t.size());
}

TEST(trie_map, erase_non_existant) {
    trie_map t{ {"bar", 1}, {"baz", 1} };
    EXPECT_EQ(0u, t.erase("foo"));
}

TEST(trie_map, erase_range) {
    trie_map t{ {"foo", 1}, {"bar", 1}, {"bax", 1}, {"bay", 1}, {"baz", 1} };

    auto first = const_cast<const trie_map&>(t).find("bar");
    auto last  = const_cast<const trie_map&>(t).find("baz");

    t.erase(first, last);

    EXPECT_EQ(1u, t.count("foo"));
    EXPECT_EQ(1u, t.count("baz"));
    ASSERT_THROW(t.at("bar") = 1, std::out_of_range);
    ASSERT_THROW(t.at("bax") = 1, std::out_of_range);
    ASSERT_THROW(t.at("bay") = 1, std::out_of_range);
}

TEST(trie_map, erase_empty_range) {
    trie_map t{ {"foo", 1}, {"bar", 1}, {"baz", 1} };

    auto first = const_cast<const trie_map&>(t).find("foo");
    auto last  = const_cast<const trie_map&>(t).find("foo");

    t.erase(first, last);

    EXPECT_EQ(1u, t.count("foo"));
    EXPECT_EQ(1u, t.count("bar"));
    EXPECT_EQ(1u, t.count("baz"));
}

TEST(trie_map, erase_returns_iterator) {
    trie_map t{ {"foo", 1}, {"bar", 1}, {"bax", 1}, {"bay", 1}, {"baz", 1} };
    auto it = t.erase(t.find("bar"));

    EXPECT_EQ("bax", it->first);
}

TEST(trie_map, extract_erases) {
    trie_map t{ {"foo", 1}, {"bar", 1}, {"baz", 1} };
    t.extract("foo");

    ASSERT_THROW(t.at("foo") = 1, std::out_of_range);
}

TEST(trie_map, extract_gives_valid_handle) {
    trie_map t{ {"foo", 1}, {"bar", 1}, {"baz", 1} };
    auto nh = t.extract("foo");

    EXPECT_EQ("foo", nh.key());
    EXPECT_EQ(1, nh.mapped());
}

TEST(trie_map, extract_reinsertion) {
    trie_map t{ {"foo", 1}, {"bar", 1}, {"baz", 1} };
    auto nh = t.extract("foo");

    t.insert(std::move(nh));
    EXPECT_EQ(1, t["foo"]);
}

TEST(trie_map, extract_reinsertion_key_change) {
    trie_map t{ {"foo", 42}, {"bar", 1}, {"baz", 1} };
    auto nh = t.extract("foo");
    nh.key() = "qux";

    t.insert(std::move(nh));
    EXPECT_EQ(42, t["qux"]);
}

TEST(trie_map, reinsertion_return_value) {
    trie_map t{ {"foo", 1}, {"bar", 1}, {"baz", 1} };

    auto nh = t.extract("foo");
    auto ret = t.insert(std::move(nh));
    EXPECT_EQ(t.find("foo"), ret.position);
    EXPECT_TRUE(ret.inserted);
    EXPECT_TRUE(ret.node.empty());

    auto empty_nh = std::move(ret.node);
    ret = t.insert(std::move(empty_nh));
    EXPECT_EQ(t.end(), ret.position);
    EXPECT_FALSE(ret.inserted);
    EXPECT_TRUE(ret.node.empty());

    trie_map s{ {"foo", 1} };
    auto existing_nh = s.extract("foo");
    ret = t.insert(std::move(existing_nh));
    EXPECT_EQ(t.find("foo"), ret.position);
    EXPECT_FALSE(ret.inserted);
    EXPECT_FALSE(ret.node.empty());
}

TEST(trie_map, merge_all) {
    trie_map t{ {"foo", 1}, {"bar", 1}, {"baz", 1} };
    trie_map s{ {"qux", 1}, {"quux", 1} };

    t.merge(s);

    EXPECT_EQ(5u, t.size());
    EXPECT_EQ(0u, s.size());
    EXPECT_EQ(1, t["qux"]);
    EXPECT_EQ(1, t["quux"]);
    ASSERT_THROW(s.at("qux") = 1, std::out_of_range);
    ASSERT_THROW(s.at("quux") = 1, std::out_of_range);
}

TEST(trie_map, merge_partial) {
    trie_map t{ {"foo", 1}, {"bar", 1}, {"baz", 1} };
    trie_map s{ {"baz", 1}, {"qux", 1} };

    t.merge(s);

    EXPECT_EQ(4u, t.size());
    EXPECT_EQ(1u, s.size());
    EXPECT_EQ(1, s["baz"]);
}

TEST(trie_map, merge_move) {
    trie_map t{ {"foo", 1}, {"bar", 1}, {"baz", 1} };
    trie_map s{ {"qux", 1}, {"quux", 1} };

    t.merge(std::move(s));

    EXPECT_EQ(5u, t.size());
    EXPECT_EQ(0u, s.size());
}

TEST(trie_map, prefixed_with) {
    std::vector<std::string> v{ "rubens", "ruber", "rubicon", "rubicundus" };
    trie_map t = fixtures::roman_trie;

    auto [first, last] = t.prefixed_with("rub");

    EXPECT_EQ(4u, std::distance(first, last));
    std::size_t i = 0;
    for (auto it = first; it != last; ++it) v[i++] = it->first;
}

TEST(trie_map, prefixed_with_empty_range) {
    trie_map t = fixtures::roman_trie;

    auto [first, last] = t.prefixed_with("rob");
    EXPECT_EQ(first, last);
}

TEST(trie_map, longest_match) {
    trie_map t{ {"foo", 1}, {"foobar", 1}, {"bar", 1} };
    auto it = t.longest_match("fooba");

    ASSERT_NE(t.end(), it);
    EXPECT_EQ("foo", it->first);
}

TEST(trie_map, longest_match_has_key) {
    trie_map t{ {"foo", 1}, {"foobar", 1}, {"bar", 1} };
    auto it = t.longest_match("foobar");

    ASSERT_NE(t.end(), it);
    EXPECT_EQ("foobar", it->first);
}

TEST(trie_map, longest_match_no_key) {
    trie_map t{ {"foo", 1}, {"foobar", 1}, {"bar", 1} };
    auto it = t.longest_match("qux");
    EXPECT_EQ(t.end(), it);
}

TEST(trie_map, longest_match_empty) {
    trie_map t;
    auto it = t.longest_match("foo");
    EXPECT_EQ(t.end(), it);
}

TEST(trie_map, equals) {
    trie_map t = fixtures::roman_trie;
    trie_map s = fixtures::roman_trie;
    EXPECT_EQ(t, s);
}

TEST(trie_map, not_equals) {
    trie_map t = fixtures::roman_trie;
    trie_map s = fixtures::roman_trie;
    s["romane"] = 2;
    EXPECT_NE(t, s);
}

TEST(trie_map, greater) {
    trie_map t = fixtures::roman_trie;
    trie_map s = fixtures::roman_trie;

    for (auto& [key, value] : s) value--;
    EXPECT_GT(t, s);
}

TEST(trie_map, greater_equals) {
    trie_map t = fixtures::roman_trie;
    trie_map s = fixtures::roman_trie;

    EXPECT_GE(t, s);
    for (auto& [key, value] : s) value--;
    EXPECT_GE(t, s);
}

TEST(trie_map, less) {
    trie_map t = fixtures::roman_trie;
    trie_map s = fixtures::roman_trie;

    for (auto& [key, value] : s) value++;
    EXPECT_LT(t, s);
}

TEST(trie_map, less_equals) {
    trie_map t = fixtures::roman_trie;
    trie_map s = fixtures::roman_trie;

    EXPECT_LE(t, s);
    for (auto& [key, value] : s) value++;
    EXPECT_LE(t, s);
}

TEST(trie_map, get_allocator) {
    using namespace std::experimental;

    using key_mapper = rmr::identity<std::size_t>;
    using allocator_type = pmr::polymorphic_allocator<std::pair<std::string, int>>;
    using trie_map = rmr::trie_map<int, 127, key_mapper, std::string, allocator_type>;

    // Create a unique allocator
    auto resource = pmr::new_delete_resource();
    auto allocator = allocator_type(resource);
    trie_map t(allocator);

    EXPECT_EQ(resource, t.get_allocator().resource());
}

TEST(trie_map, get_key_map) {
    struct map {
        int v;
        std::size_t operator()(std::size_t n) { return n; }
    };

    map m{42};
    rmr::trie_map<int, 127, map> t(m);
    EXPECT_EQ(m.v, t.key_map().v);
}

TEST(trie_map, radix) {
    trie_map t;
    EXPECT_EQ(127u, t.radix());
    EXPECT_EQ(127u, trie_map::radix());
}

TEST(trie_map, DISABLED_does_not_leak) {
    EXPECT_TRUE(false) << "Not implemented";
}
