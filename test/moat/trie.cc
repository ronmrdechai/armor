#include <gtest/gtest.h>

#include <moat/trie.h>

using trie = moat::trie<int, 127>;

TEST(trie, write_and_read) {
    trie t;
    t["foo"] = 1;
    t["bar"] = 2;
    EXPECT_EQ(1, t["foo"]);
    EXPECT_EQ(2, t["bar"]);
}

TEST(trie, safe_access_read) {
    trie t;
    t["foo"] = 1;
    t["bar"] = 2;
    EXPECT_EQ(1, t.at("foo"));
    EXPECT_EQ(2, t.at("bar"));
}

TEST(trie, safe_access_write) {
    trie t;
    t["foo"] = 1;
    t.at("foo") = 2;
    EXPECT_EQ(2, t["foo"]);
}

TEST(trie, safe_access_throws) {
    trie t;

    ASSERT_THROW(t.at("foo"), std::out_of_range);
    ASSERT_THROW(t.at("foo") = 1, std::out_of_range);
}

TEST(trie, default_is_empty) {
    trie t;
    EXPECT_TRUE(t.empty());
}

TEST(trie, not_empty_after_write) {
    trie t;
    t["foo"] = 1;
    EXPECT_FALSE(t.empty());
}

TEST(trie, empty_after_clear) {
    trie t;
    t["foo"] = 1;
    t.clear();
    EXPECT_TRUE(t.empty());
}

TEST(trie, default_size_is_zero) {
    trie t;
    EXPECT_EQ(0u, t.size());
}

TEST(trie, size_increase_after_write) {
    trie t;
    t["foo"] = 1;
    EXPECT_EQ(1u, t.size());
    t["bar"] = 1;
    EXPECT_EQ(2u, t.size());
}

TEST(trie, default_count_is_zero) {
    trie t;
    EXPECT_EQ(0u, t.count("foo"));
}

TEST(trie, count_increase_after_write) {
    trie t;
    t["foo"] = 1;
    EXPECT_EQ(1u, t.count("foo"));
}

TEST(trie, read_iteration) {
    std::vector<std::string> strings{ "bar", "baz", "foo" };
    trie t;

    for (std::string& s : strings) t[s] = 42;

    std::size_t i = 0;
    for (auto& [key, value] : t) {
        EXPECT_EQ(strings[i++], key);
        EXPECT_EQ(42, value);
    }
}

TEST(trie, write_iteration) {
    std::vector<std::string> strings{ "bar", "baz", "foo" };
    trie t;

    for (std::string& s : strings) t[s] = 42;
    for (auto& [_, value] : t) value = 0;
    for (std::string& s : strings) EXPECT_EQ(0, t[s]);
}

TEST(trie, find_existant) {
    trie t;
    t["foo"] = 1;
    auto it = t.find("foo");
    EXPECT_EQ(t.begin(), it);
}

TEST(trie, find_non_existant) {
    trie t;
    EXPECT_EQ(t.end(), t.find("foo"));
}

TEST(trie, try_emplace_and_access) {
    trie t;
    t.try_emplace("foo", 1);
    EXPECT_EQ(1, t["foo"]);
}

TEST(trie, try_emplace_twice) {
    trie t;

    bool emplaced;
    typename trie::iterator it1, it2;
    std::tie(it1, emplaced) = t.try_emplace("foo", 1);
    EXPECT_TRUE(emplaced);

    std::tie(it2, emplaced) = t.try_emplace("foo", 2);
    EXPECT_FALSE(emplaced);
    EXPECT_EQ(it1, it2);
    EXPECT_EQ(1, t["foo"]);
}

TEST(trie, insert_and_access) {
    trie t;
    t.insert( {"foo", 1} );
    EXPECT_EQ(1, t["foo"]);
}

TEST(trie, insert_twice) {
    trie t;

    bool inserted;
    typename trie::iterator it1, it2;
    std::tie(it1, inserted) = t.insert( {"foo", 1} );
    EXPECT_TRUE(inserted);

    std::tie(it2, inserted) = t.insert( {"foo", 2} );
    EXPECT_FALSE(inserted);
    EXPECT_EQ(it1, it2);
    EXPECT_EQ(1, t["foo"]);
}

TEST(trie, insert_or_assign_and_access) {
    trie t;
    t.insert_or_assign( {"foo", 1} );
    EXPECT_EQ(1, t["foo"]);
}

TEST(trie, insert_or_assign_twice) {
    trie t;

    bool inserted;
    typename trie::iterator it1, it2;
    std::tie(it1, inserted) = t.insert_or_assign( {"foo", 1} );
    EXPECT_TRUE(inserted);

    std::tie(it2, inserted) = t.insert_or_assign( {"foo", 2} );
    EXPECT_FALSE(inserted);
    EXPECT_EQ(it1, it2);
    EXPECT_EQ(2, t["foo"]);
}

TEST(trie, insert_range) {
    trie t;
    std::vector<typename trie::value_type> v{
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

TEST(trie, insert_list) {
    trie t;
    std::vector<typename trie::value_type> v{
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

TEST(trie, equality) {
    trie t1;
    t1["foo"] = 1;
    t1["bar"] = 2;
    trie t2;
    t2["foo"] = 1;
    t2["bar"] = 2;
    EXPECT_TRUE(t1 == t2);
}

TEST(trie, key_inequlity) {
    trie t1;
    t1["foo"] = 1;
    t1["bar"] = 2;
    trie t2;
    t2["foo"] = 1;
    t2["baz"] = 2;
    EXPECT_TRUE(t1 != t2);
}

TEST(trie, value_inequlity) {
    trie t1;
    t1["foo"] = 1;
    t1["bar"] = 2;
    trie t2;
    t2["foo"] = 1;
    t2["bar"] = 3;
    EXPECT_TRUE(t1 != t2);
}

TEST(trie, key_value_inequlity) {
    trie t1;
    t1["foo"] = 1;
    t1["bar"] = 2;
    trie t2;
    t2["foo"] = 1;
    t2["baz"] = 3;
    EXPECT_TRUE(t1 != t2);
}

TEST(trie, copy_constructor) {
    trie t1;
    t1["foo"] = 1;
    t1["bar"] = 2;

    trie t2(t1);
    EXPECT_EQ(t1, t2);
}

TEST(trie, change_original_after_copy) {
    trie t1;
    t1["foo"] = 1;
    t1["bar"] = 2;

    trie t2(t1);

    t1["foo"] = 3;
    EXPECT_NE(t1, t2);
}

TEST(trie, change_copy_after_copy) {
    trie t1;
    t1["foo"] = 1;
    t1["bar"] = 2;

    trie t2(t1);

    t2["foo"] = 3;
    EXPECT_NE(t1, t2);
}

TEST(trie, iterator_constructor) {
    std::vector<std::pair<std::string, int>> v{
        { "bar", 1 }, { "baz", 2 }, { "foo", 3 }
    };

    trie t(v.begin(), v.end());

    for (auto& [key, value] : v) {
        EXPECT_EQ(value, t[key]);
    }
}

TEST(trie, initializer_list_constructor) {}

TEST(trie, does_not_leak) {}
