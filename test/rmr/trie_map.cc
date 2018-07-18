#include <gtest/gtest.h>

#include <rmr/trie_map.h>

using trie_map = rmr::trie_map<int, 127>;

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

TEST(trie_map, insert_and_access) {
    trie_map t;
    t.insert( {"foo", 1} );
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

TEST(trie_map, equality) {
    trie_map t;
    t["foo"] = 1;
    t["bar"] = 2;
    trie_map s;
    s["foo"] = 1;
    s["bar"] = 2;
    EXPECT_TRUE(t == s);
}

TEST(trie_map, key_inequlity) {
    trie_map t;
    t["foo"] = 1;
    t["bar"] = 2;
    trie_map s;
    s["foo"] = 1;
    s["baz"] = 2;
    EXPECT_TRUE(t != s);
}

TEST(trie_map, value_inequlity) {
    trie_map t;
    t["foo"] = 1;
    t["bar"] = 2;
    trie_map s;
    s["foo"] = 1;
    s["bar"] = 3;
    EXPECT_TRUE(t != s);
}

TEST(trie_map, key_value_inequlity) {
    trie_map t;
    t["foo"] = 1;
    t["bar"] = 2;
    trie_map s;
    s["foo"] = 1;
    s["baz"] = 3;
    EXPECT_TRUE(t != s);
}

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
}

TEST(trie_map, erase_empty) {
}

TEST(trie_map, erase_size_drop) {
}

TEST(trie_map, erase_non_existant) {
}

TEST(trie_map, extract_erases) {
}

TEST(trie_map, extract_gives_valid_handle) {
}

TEST(trie_map, extract_reinsertion) {
}

TEST(trie_map, extract_reinsertion_key_change) {
}

TEST(trie_map, node_type_properties) {
}

TEST(trie_map, DISABLED_does_not_leak) {
    EXPECT_TRUE(false) << "Not implemented";
}
