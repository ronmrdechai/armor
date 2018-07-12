#pragma once

#include <iterator>
#include <stdexcept>
#include <string>
#include <tuple>

namespace moat {

/**
 * An implementation of an R-way Trie.
 * A Trie is an insertion efficient string-based associative container.
 * Insertion is linearly dependent on the size of the key. Tries also support
 * prefix-based queries such as `all keys with prefix' or `key with longest
 * prefix of'.
 *
 * @tparam T  The value of keys contained in this Trie.
 * @tparam R  The radix of this trie's strings.
 * @tparam F  A function to map from a key character to a value in the range
 *            (0, R].
 *
 * For examples of radix_trie template parameters, see the following aliases:
 * - ascii_trie
 * - lowercase_trie
 * - uppercase_trie
 */
template <
    typename T,
    std::size_t R,
    std::size_t (&F)(std::size_t),
    typename Key = std::string,
    typename Allocator = std::allocator<T>
>
class radix_trie {
public:
    using key_type        = Key;
    using char_type       = typename key_type::value_type;
    using mapped_type     = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using allocator_type  = Allocator;
    using reference       = mapped_type&;
    using const_reference = const mapped_type&;
    using pointer         = mapped_type*;
    using const_pointer   = const mapped_type*;

    struct node_type {
        std::array<node_type*, R> children = { nullptr };
        node_type*                parent = nullptr;
        mapped_type*              value = nullptr;

        void destroy(allocator_type& allocator) {
            if (value != nullptr) {
                allocator.deallocate(value, 1);
                value = nullptr;
            }

            for (auto& child : children) {
                if (child != nullptr) {
                    child->destroy(allocator);
                    delete child;
                    child = nullptr;
                }
            }
        }
    };

    radix_trie() = default;
    radix_trie(radix_trie&&) = default;
    radix_trie& operator=(radix_trie&&) = default;

    radix_trie(allocator_type allocator) : allocator_(std::move(allocator)) {}

    ~radix_trie() { root_.destroy(allocator_); }

    mapped_type& operator[](const key_type& key) {
        insert_key(&root_, nullptr, key.begin(), key.end());
        return *find_key<false>(&root_, key.begin(), key.end());
    }

    mapped_type& at(const key_type& key) {
        return *find_key<true>(&root_, key.begin(), key.end());
    }
    const mapped_type& at(const key_type& key) const {
        return *find_key<true>(&root_, key.begin(), key.end());
    }

    /* using iterator = ...; */
    /* using reverse_iterator = std::reverse_iterator<iterator>; */
    /* using const_iterator = ...; */
    /* using const_reverse_iterator = std::reverse_iterator<const_iterator>; */

    /* iterator begin(); */
    /* const_iterator begin() const; */
    /* const_iterator cbegin() const; */
    /* reverse_iterator rbegin(); */
    /* reverse_const_iterator rbegin() const; */
    /* reverse_const_iterator crbegin() const; */
    /* iterator end(); */
    /* const_iterator end() const; */
    /* const_iterator cend() const; */
    /* reverse_iterator rend(); */
    /* reverse_const_iterator rend() const; */
    /* reverse_const_iterator crend() const; */

    bool empty() const {
        return std::all_of(
            root_.children.begin(),
            root_.children.end(),
            [](node_type* child) { return child == nullptr; }
        );
    }

    size_type size() const {
        size_type c = 0;
        count_keys(&root_, c);
        return c;
    }

    void clear() {
        root_.destroy(allocator_);
    }

    size_type count(const key_type& key) const {
        try {
            find_key<true>(&root_, key.begin(), key.end());
            return 1;
        } catch (std::out_of_range&) {
            return 0;
        }
    }

    // TODO:
    // - copy constructor
    // - iterators
    // - prefix search
    // - longest matching prefix
    // - insert, insert_or_assign, emplace, try_emplace, erase, swap, extract, merge
    // - operator==, operator!=, operator<, operator<=, operator>, operator>=

private:
    node_type* insert_key(
        node_type* root,
        node_type* parent,
        typename key_type::const_iterator cur,
        typename key_type::const_iterator last
    ) {
        if (root == nullptr) {
            root = new node_type{};
            root->parent = parent;
        }

        if (cur == last) {
            if (root->value == nullptr) root->value = allocator_.allocate(1);
        } else {
            auto& child = root->children[F(*cur)];
            child = insert_key(child, root, ++cur, last);
        }

        return root;
    }

    template <bool SAFE>
    static mapped_type* find_key(
        const node_type* root,
        typename key_type::const_iterator cur,
        typename key_type::const_iterator last
    ) {
        if constexpr (SAFE) {
            if (root == nullptr) {
                throw std::out_of_range("moat::radix_trie::at");
            }
        }

        if (cur == last) {
            if constexpr (SAFE) {
                if (root->value == nullptr) {
                    throw std::out_of_range("moat::radix_trie::at");
                }
            }
            return root->value;
        }

        auto& child = root->children[F(*cur)];
        return find_key<SAFE>(child, ++cur, last);
    }

    static void count_keys(const node_type* root, size_type& count) {
        for (auto& child : root->children) {
            if (child != nullptr) {
                count_keys(child, count);
                if (child->value != nullptr) count += 1;
            }
        }
    }

    node_type      root_;
    allocator_type allocator_;
};

namespace trie {

constexpr std::size_t identity(std::size_t n) { return n; }
template <char S>
constexpr std::size_t count_from(std::size_t n) { return n - S; }

} // namespace trie

/// A Trie mapping strings of ASCII characters only.
template <typename T>
using ascii_trie = radix_trie<T, 127, trie::identity>;

/// A Trie mapping strings of lowercase letters only.
template <typename T>
using lowercase_trie = radix_trie<T, 26, trie::count_from<'a'>>;

/// A Trie mapping strings of uppercase letters only.
template <typename T>
using uppercase_trie = radix_trie<T, 26, trie::count_from<'A'>>;

} // namespace moat
