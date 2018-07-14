#pragma once

#include <iterator>
#include <stdexcept>
#include <string>
#include <stack>

#include <moat/utility.h>

namespace moat {

/**
 * An implementation of an R-way Trie.
 * A Trie is an insertion efficient string-based associative container.
 * Insertion is linearly dependent on the size of the key. Tries also support
 * prefix-based queries such as 'all keys with prefix' or 'key with longest
 * prefix of'.
 *
 * @tparam T          The value of keys contained in this trie.
 * @tparam R          The radix of this trie's strings.
 * @tparam F          A function object to map from a key character to a value
 *                    within the range (0, R].
 * @tparam Key        The type of the trie keys.
 * @tparam Allocator  An allocator to type use when inserting or removing
 *                    values.
 *
 * Examples of trie instantiations are:
 * - `using ascii_trie = trie<T, 127;`
 * - `using lowercase_trie = trie<T, 26, count_from<char, 'a'>;`
 * - `using uppercase_trie = trie<T, 26, count_from<char, 'A'>;`
 * - `using dna_trie = trie<T, 4, indexed<char, 'A', 'C', 'G', 'T'>;`
 */
template <
    typename T,
    std::size_t R,
    typename F = identity<std::size_t>,
    typename Key = std::string,
    typename Allocator = std::allocator<std::pair<const Key, T>>
>
class trie {
    using allocator_traits = std::allocator_traits<Allocator>;
public:
    using key_type        = Key;
    using char_type       = typename key_type::value_type;
    using mapped_type     = T;
    using value_type      = std::pair<const key_type, mapped_type>;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using key_map         = F;
    using allocator_type  = Allocator;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using pointer         = typename allocator_traits::pointer;
    using const_pointer   = typename allocator_traits::const_pointer;

    struct node_type {
        std::array<node_type*, R> children = { nullptr };
        value_type*                  value = nullptr;
        node_type*                  parent = nullptr;

        void destroy(allocator_type& allocator) {
            if (value != nullptr) {
                allocator_traits::deallocate(allocator, value, 1);
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

    trie() {
        root_.parent = &base_;
        base_.children[0] = &root_;
    }
    trie(trie&&) = default;
    trie(allocator_type allocator) : allocator_(std::move(allocator)) {}

    trie& operator=(trie&&) = default;

    ~trie() { root_.destroy(allocator_); }

    mapped_type& operator[](const key_type& key) {
        insert_key(&root_, nullptr, key);
        return find_key<false>(&root_, key.begin(), key.end())->second;
    }

    mapped_type& at(const key_type& key) {
        return find_key<true>(&root_, key.begin(), key.end())->second;
    }
    const mapped_type& at(const key_type& key) const {
        return find_key<true>(&root_, key.begin(), key.end())->second;
    }

    template <typename NodeT>
    class generic_iterator {
    public:
        using difference_type   = std::ptrdiff_t;
        using value_type        = typename trie::value_type;
        using reference         = typename trie::reference;
        using pointer           = typename trie::pointer;
        using iterator_category = std::forward_iterator_tag;

        generic_iterator() = default;
        generic_iterator(NodeT* node) : node_(std::move(node)) {}
        generic_iterator(const generic_iterator&) = default;
        generic_iterator& operator=(const generic_iterator&) = default;

        generic_iterator& operator++() {
            do {
                *this = next(std::move(*this));
            } while(this->node_->value == nullptr && !positions_.empty());
            return *this;
        }

        generic_iterator operator++(int) {
            generic_iterator it = next(*this);
            while (it.node_->value == nullptr && !positions_.empty()) {
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

        friend void swap(generic_iterator& lhs, generic_iterator& rhs) {
            std::swap(lhs.node_, rhs.node_);
            std::swap(lhs.positions_, rhs.positions_);
        }

    private:
        static std::pair<generic_iterator, bool>
        step_down(generic_iterator it) {
            for (size_type pos = 0; pos < R; ++pos) {
                if (it.node_->children[pos] != nullptr) {
                    it.node_ = it.node_->children[pos];
                    it.positions_.push(pos);
                    return {it, true};
                }
            }
            return {it, false};
        }

        static std::pair<generic_iterator, bool>
        step_right(generic_iterator it) {
            if (it.positions_.empty()) return {it, false};

            for (size_type pos = it.positions_.top() + 1; pos < R; ++pos) {
                node_type* parent = it.node_->parent;
                if (parent->children[pos] != nullptr) {
                    it.node_ = parent->children[pos];
                    it.positions_.top() = pos;
                    return {it, true};
                }
            }
            return {it, false};
        }

        static std::pair<generic_iterator, bool>
        step_up(generic_iterator it) {
            it.node_ = it.node_->parent;
            it.positions_.pop();
            return step_right(it);
        }

        static generic_iterator next(generic_iterator it) {
            bool stepped;

            std::tie(it, stepped) = step_down(std::move(it));
            if (stepped) return it;
            std::tie(it, stepped) = step_right(std::move(it));
            if (stepped) return it;

            while (!it.positions_.empty()) {
                std::tie(it, stepped) = step_up(std::move(it));
                if (stepped) return it;
            }
            return it;
        }

        NodeT*                node_;
        std::stack<size_type> positions_;
    };

    using iterator = generic_iterator<node_type>;
    using const_iterator = generic_iterator<const node_type>;

    iterator begin() {
        if (empty()) return end();
        iterator it(&base_);
        return ++it;
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator cbegin() const {
        if (empty()) return cend();
        const_iterator it(&base_);
        return ++it;
    }

    iterator end() {
        return iterator(&base_);
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cend() const {
        return const_iterator(&base_);
    }

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
    // - prefix search
    // - longest matching prefix
    // - insert, insert_or_assign, emplace, try_emplace, erase, swap, extract, merge
    // - operator==, operator!=, operator<, operator<=, operator>, operator>=

private:
    value_type* allocate_value(const key_type& key) {
        value_type* value = allocator_traits::allocate(allocator_, 1);
        allocator_traits::construct(
            allocator_,
            value,
            std::piecewise_construct,
            std::tuple<const key_type&>(key),
            std::tuple<>()
        );
        return value;
    }

    node_type* insert_key(
        node_type* root,
        node_type* parent,
        const key_type& key,
        typename key_type::size_type index = 0
    ) {
        if (root == nullptr) {
            root = new node_type{};
            root->parent = parent;
        }

        if (index == key.size()) {
            if (root->value == nullptr) root->value = allocate_value(key);
        } else {
            auto& child = root->children[key_map_(key[index])];
            child = insert_key(child, root, key, index + 1);
        }

        return root;
    }

    template <bool SAFE>
    value_type* find_key(
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

    node_type      base_;
    node_type      root_;
    allocator_type allocator_;
    key_map        key_map_;
};

} // namespace moat
