#pragma once

#include <iterator>
#include <stdexcept>
#include <string>
#include <stack>

namespace moat {

namespace utility {

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

} // namespace utility

/**
 * An implementation of an R-way Trie.
 * A Trie is an insertion efficient string-based associative container.
 * Insertion is linearly dependent on the size of the key. Tries also support
 * prefix-based queries such as 'all keys with prefix' or 'key with longest
 * prefix of'.
 *
 * @tparam T          The value of keys contained in this Trie.
 * @tparam R          The radix of this trie's strings.
 * @tparam F          A functor to map from a key character to a value within
 *                    the range (0, R].
 * @tparam Key        The type of map keys.
 * @tparam Allocator  An allocator to type use when inserting or removing
 *                    values.
 *
 * Examples of trie instantiations are:
 * - using ascii_trie = trie<T, 127;
 * - using lowercase_trie = trie<T, 26, moat::count_from<char, 'a'>;
 * - using uppercase_trie = trie<T, 26, moat::count_from<char, 'A'>;
 * - using dna_trie = trie<T, 4, moat::indexed<char, 'A', 'C', 'G', 'T'>;
 */
template <
    typename T,
    std::size_t R,
    typename F = utility::identity<std::size_t>,
    typename Key = std::string,
    typename Allocator = std::allocator<T>
>
class trie {
public:
    using key_type        = Key;
    using char_type       = typename key_type::value_type;
    using mapped_type     = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using key_map         = F;
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

    trie() = default;
    trie(trie&&) = default;
    trie& operator=(trie&&) = default;

    trie(allocator_type allocator) : allocator_(std::move(allocator)) {}

    ~trie() { root_.destroy(allocator_); }

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

    template <typename NodeT>
    class generic_iterator {
    public:
        generic_iterator(NodeT* node, std::stack<size_type> positions) :
            node_(node), positions_(std::move(positions))
        {}
        generic_iterator(const generic_iterator&) = default;
        generic_iterator& operator=(const generic_iterator&) = default;

        generic_iterator& operator++() {
            do {
                *this = next(std::move(*this));
            } while(this->node_->value == nullptr);
            return *this;
        }

        generic_iterator operator++(int) {
            generic_iterator it = next(*this);
            while (it.node_->value == nullptr) {
                it = next(it);
            }
            return it;
        }

        reference operator*() const {
            return *(node_->value);
        }

        pointer operator->() const {
            return node_->value;
        }

        bool operator==(const generic_iterator& other) {
            return (node_ == other.node_) && (positions_ == other.positions_);
        }

        bool operator!=(const generic_iterator& other) {
            return !(*this == other);
        }
        // friend void swap(generic_iterator& lhs, generic_iterator& rhs);

    private:
        static generic_iterator next(generic_iterator cur);

        NodeT*                node_;
        std::stack<size_type> positions_;
    };

    using iterator = generic_iterator<node_type>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_iterator = generic_iterator<const node_type>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

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
            at(key);
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
            auto& child = root->children[key_map_(*cur)];
            child = insert_key(child, root, ++cur, last);
        }

        return root;
    }

    template <bool SAFE>
    mapped_type* find_key(
        const node_type* root,
        typename key_type::const_iterator cur,
        typename key_type::const_iterator last
    ) const {
        if constexpr (SAFE) {
            if (root == nullptr) {
                throw std::out_of_range("moat::trie::at");
            }
        }

        if (cur == last) {
            if constexpr (SAFE) {
                if (root->value == nullptr) {
                    throw std::out_of_range("moat::trie::at");
                }
            }
            return root->value;
        }

        auto& child = root->children[key_map_(*cur)];
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
    key_map        key_map_;
};

/// A Trie mapping strings of ASCII characters only.
template <typename T>
using ascii_trie = trie<T, 127>;

/// A Trie mapping strings of lowercase letters only.
template <typename T>
using lowercase_trie = trie<T, 26, utility::count_from<std::size_t, 'a'>>;

/// A Trie mapping strings of uppercase letters only.
template <typename T>
using uppercase_trie = trie<T, 26, utility::count_from<std::size_t, 'A'>>;

} // namespace moat
