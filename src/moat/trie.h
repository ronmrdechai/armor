#pragma once

#include <iterator>
#include <initializer_list>
#include <type_traits>
#include <stdexcept>
#include <string>
#include <stack>

#include <moat/functors.h>

namespace moat {

namespace detail {

template <typename...> struct rebind;
template <template <typename> typename Type, typename Inner, typename New>
struct rebind<Type<Inner>, New> {
    using type = Type<New>;
};

} // namespace detail

/**
 * An implementation of an R-way Trie.
 * A Trie is an insertion efficient string-based associative container.
 * Insertion is linearly dependent on the size of the key. Tries also support
 * prefix-based queries such as 'all keys with prefix' or 'key with longest
 * prefix of'.
 *
 * @tparam T          The value of keys contained in this trie.
 * @tparam R          The radix of this trie's strings.
 * @tparam KeyMapper  A function object to map from a key character to a value
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
    typename KeyMapper = identity<std::size_t>,
    typename Key = std::string,
    typename Allocator = std::allocator<std::pair<const Key, T>>
>
class trie {
    using allocator_traits = std::allocator_traits<Allocator>;
    static_assert(
        std::is_invocable_r_v<std::size_t, KeyMapper, std::size_t>, 
        "KeyMapper is not invokable on std::size_t or does not return std::size_t"
    );
public:
    using key_type        = Key;
    using char_type       = typename key_type::value_type;
    using mapped_type     = T;
    using value_type      = std::pair<const key_type, mapped_type>;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using key_mapper      = KeyMapper;
    using allocator_type  = Allocator;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using pointer         = typename allocator_traits::pointer;
    using const_pointer   = typename allocator_traits::const_pointer;

    class node_type;
private:
    using handle_allocator_type = typename detail::rebind<allocator_type, node_type>::type;
    using handle_allocator_traits = std::allocator_traits<handle_allocator_type>;
public:
    class node_type {
    public:
        using key_type       = trie::key_type;
        using mapped_type    = trie::mapped_type;
        using value_type     = trie::value_type;
        using allocator_type = trie::allocator_type;

        constexpr node_type(allocator_type allocator) :
            value_(nullptr), allocator_(std::move(allocator))
        {} 
        node_type(node_type&&) = default;

        bool             empty() const { return value == nullptr; }
        explicit operator bool() const { return !empty(); }

        const key_type& key()    const { return  value_->first; }
        mapped_type&    mapped() const { return  value_->second; }
        value_type&     value()  const { return *value_; }

        void swap(node_type& nh) {
            std::swap(value_,     nh.value_);
            std::swap(allocator_, nh.allocator_);
        }
        friend void swap(node_type& x, node_type& y) {
            x.swap(y);
        }

    private:
        pointer                              value_;
        std::optional<handle_allocator_type> allocator_;

        friend class trie;
    };
private:
    struct trie_node;
    using node_allocator_type = typename detail::rebind<allocator_type, trie_node>::type;
    using node_allocator_traits = std::allocator_traits<node_allocator_type>;
    struct trie_node {
        std::array<trie_node*, R> children_ = { nullptr };
        trie_node*                parent_   = nullptr;
        node_type*                handle_   = nullptr;

        void destroy(
            handle_allocator_type& handle_allocator, node_allocator_type& node_allocator
        ) {
            if (handle_ != nullptr) {
                handle_allocator_traits::deallocate(handle_allocator, handle_, 1);
                handle_ = nullptr;
            }

            for (auto& child : children_) {
                if (child != nullptr) {
                    child->destroy(handle_allocator, node_allocator);
                    node_allocator_traits::deallocate(node_allocator, child, 1);
                    child = nullptr;
                }
            }
        }
    };

public:
    trie() { init_base_root(); }
    explicit trie(key_mapper f, allocator_type allocator = allocator_type()) :
        key_map_(std::move(f)), allocator_(std::move(allocator))
    { init_base_root(); }
    explicit trie(allocator_type allocator) :
        allocator_(std::move(allocator)) 
    { init_base_root(); }

    trie(trie&&) = default;
    trie(trie&& other, allocator_type allocator) :
        base_(std::move(other.base_)),
        root_(std::move(other.root_)),
        allocator_(std::move(allocator)),
        key_map_(std::move(other.key_map_))
    {}

    template <typename InputIt>
    trie(
        InputIt first,
        InputIt last,
        key_mapper f = key_mapper(),
        allocator_type allocator = allocator_type()
    ) : allocator_(std::move(allocator)), key_map_(std::move(f)) {
        init_base_root();
        for (auto cur = first; cur != last; ++cur) insert(*cur);
    }

    trie(
        std::initializer_list<value_type> init,
        key_mapper f = key_mapper(),
        allocator_type allocator = allocator_type()
    ) : trie(init.begin(), init.end(), std::move(f), std::move(allocator)) {}
    trie(std::initializer_list<value_type> init, allocator_type allocator) :
        trie(init.begin(), init.end(), key_mapper(), std::move(allocator))
    {}

    trie(const trie& other) : trie(other.begin(), other.end()) {}
    trie(const trie& other, allocator_type allocator) :
        trie(other.begin(), other.end(), std::move(allocator))
    {}

    ~trie() { root_.destroy(handle_allocator_, node_allocator_); }

    trie& operator=(trie&&) = default;
    trie& operator=(const trie&& other) {
        clear();
        insert(other.begin(), other.end());
    }

    mapped_type& operator[](const key_type& key) {
        insert_handle(
            &root_, &base_, key, 0, 
            make_handle(std::piecewise_construct, std::tuple<const key_type&>(key), std::tuple<>())
        );
        iterator it = find(key);
        return it->second;
    }

    mapped_type& at(const key_type& key) {
        iterator it = find(key);
        if (it == end()) throw std::out_of_range("moat::trie::at");
        return it->second;
    }
    const mapped_type& at(const key_type& key) const {
        const_iterator it = find(key);
        if (it == end()) throw std::out_of_range("moat::trie::at");
        return it->second;
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
            } while(this->node_->handle_ == nullptr && !positions_.empty());
            return *this;
        }

        generic_iterator operator++(int) {
            generic_iterator it = next(*this);
            while (it.node_->handle_ == nullptr && !positions_.empty()) {
                it = next(it);
            }
            return it;
        }

        reference operator*() const {
            return *(node_->handle_->value_);
        }

        pointer operator->() const {
            return node_->handle_->value_;
        }

        bool operator==(const generic_iterator& other) const {
            return (node_ == other.node_) && (positions_ == other.positions_);
        }

        bool operator!=(const generic_iterator& other) const {
            return !(*this == other);
        }

        friend void swap(generic_iterator& lhs, generic_iterator& rhs) {
            std::swap(lhs.node_, rhs.node_);
            std::swap(lhs.positions_, rhs.positions_);
        }

    private:
        NodeT*                node_;
        std::stack<size_type> positions_;

        static std::pair<generic_iterator, bool>
        step_down(generic_iterator it) {
            for (size_type pos = 0; pos < R; ++pos) {
                if (it.node_->children_[pos] != nullptr) {
                    it.node_ = it.node_->children_[pos];
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
                trie_node* parent = it.node_->parent_;
                if (parent->children_[pos] != nullptr) {
                    it.node_ = parent->children_[pos];
                    it.positions_.top() = pos;
                    return {it, true};
                }
            }
            return {it, false};
        }

        static std::pair<generic_iterator, bool>
        step_up(generic_iterator it) {
            it.node_ = it.node_->parent_;
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

        friend class trie;
    };

    using iterator = generic_iterator<trie_node>;
    using const_iterator = generic_iterator<const trie_node>;

    iterator begin() {
        if (empty()) return end();
        iterator it(&base_);
        return ++it;
    }
    const_iterator begin() const { return cbegin(); }
    const_iterator cbegin() const {
        if (empty()) return cend();
        const_iterator it(&base_);
        return ++it;
    }

    iterator end() { return iterator(&base_); }
    const_iterator end() const { return cend(); }
    const_iterator cend() const { return const_iterator(&base_); }

    bool empty() const {
        return std::all_of(
            root_.children_.begin(),
            root_.children_.end(),
            [](trie_node* child) { return child == nullptr; }
        );
    }

    size_type size() const {
        size_type c = 0;
        count_keys(&root_, c);
        return c;
    }

    void clear() {
       root_.destroy(handle_allocator_, node_allocator_);
    }

    size_type count(const key_type& key) const {
        try {
            at(key);
            return 1;
        } catch (std::out_of_range&) {
            return 0;
        }
    }

    iterator find(const key_type& key) {
        return find_key(
            root_iterator(), end(), key_map_, key.begin(), key.end()
        );
    }
    const_iterator find(const key_type& key) const {
        return find_key(
            root_iterator(), end(), key_map_, key.begin(), key.end()
        );
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        node_type* nh = make_handle(std::forward<Args>(args)...);

        iterator it = find(nh->key());
        if (it != end()) {
            handle_allocator_traits::deallocate(handle_allocator_, nh, 1);
            return {it, false};
        }

        insert_handle(&root_, &base_, nh->key(), 0, nh);
        return {find(nh->key()), true};
    }

    template <typename... Args>
    std::pair<iterator, bool> try_emplace(const key_type& key, Args&&... args) {
        iterator it = find(key);
        if (it == end()) return emplace(value_type(
            std::piecewise_construct,
            std::forward_as_tuple(key),
            std::forward_as_tuple(args...)
        ));
        else return {it, false};
    }

    template <typename... Args>
    std::pair<iterator, bool> try_emplace(key_type&& key, Args&&... args) {
        iterator it = find(key);
        if (it == end()) return emplace(value_type(
            std::piecewise_construct,
            std::forward_as_tuple(std::move(key)),
            std::forward_as_tuple(args...)
        ));
        else return {it, false};
    }

    std::pair<iterator, bool> insert(const value_type& value) {
        iterator it = find(value.first);
        if (it != end()) return {it, false};

        insert_handle(&root_, &base_, value.first, 0, make_handle(value));
        return {find(value.first), true};
    }
    template <typename InputIt>
    void insert(InputIt first, InputIt last) {
        for (auto it = first; it != last; ++it) insert(*it);
    }
    void insert(std::initializer_list<value_type> init) {
        insert(init.begin(), init.end());
    }

    std::pair<iterator, bool> insert_or_assign(const value_type& value) {
        auto [it, inserted] = insert(value);
        if (!inserted) it->second = value.second;
        return {it, inserted};
    }

    // TODO:
    // - prefix search
    // - longest matching prefix
    // - erase, swap, extract, merge
    // - deduction guides
    
    bool operator==(const trie& other) const {
        for (
            auto it1 = begin(), it2 = other.begin();
            it1 != end() && it2 != other.end();
            ++it1, ++it2
        ) {
            if (it1->first  != it2->first ) return false;
            if (it1->second != it2->second) return false;
        }
        return true;
    }

    bool operator!=(const trie& other) const {
        return !(*this == other);
    }

    void swap(trie& other) {
        using std::swap;

        swap(base_, other.base_);
        swap(root_, other.root_);
        swap(allocator_, other.allocator_);
        swap(key_map_, other.key_map_);
    }

    friend void swap(trie& lhs, trie& rhs) {
        lhs.swap(rhs);
    }

    allocator_type get_allocator() const { return allocator_; }
    key_mapper     key_map()       const { return key_map_; }

private:
    trie_node             base_;
    trie_node             root_;
    allocator_type        allocator_;
    node_allocator_type   node_allocator_;
    handle_allocator_type handle_allocator_;
    key_mapper            key_map_;

    void init_base_root() {
        root_.parent_ = &base_;
        base_.children_[0] = &root_;
    }

    template <typename... Args>
    node_type* make_handle(Args&&... args) {
        node_type* handle = handle_allocator_traits::allocate(handle_allocator_, 1);
        handle_allocator_traits::construct(handle_allocator_, handle, handle_allocator_);
        handle->value_ = allocator_traits::allocate(allocator_, 1);
        allocator_traits::construct(allocator_, handle->value_, std::forward<Args>(args)...);
        return handle;
    }

    trie_node* make_trie_node() {
        trie_node* node = node_allocator_traits::allocate(node_allocator_, 1);
        node_allocator_traits::construct(node_allocator_, node);
        return node;
    }

    trie_node* insert_handle(
        trie_node* root,
        trie_node* parent,
        const key_type& key,
        size_type index,
        node_type* nh
    ) {
        if (root == nullptr) {
            root = make_trie_node();
            root->parent_ = parent;
        }

        if (index == key.size()) {
            if (root->handle_ == nullptr) root->handle_ = nh;
            else                          handle_allocator_traits::deallocate(handle_allocator_, nh, 1);
        } else {
            auto& child = root->children_[key_map_(key[index])];
            child = insert_handle(child, root, key, index + 1, nh);
        }

        return root;
    }

    iterator root_iterator() {
        iterator it(&root_);
        it.positions_.push(0);
        return it;
    }

    const_iterator root_iterator() const {
        const_iterator it(&root_);
        it.positions_.push(0);
        return it;
    }

    template <typename It>
    static It find_key(
        It it,
        It end,
        const key_mapper& f,
        typename key_type::const_iterator cur,
        typename key_type::const_iterator last
    ) {
        if (it.node_ == nullptr) return end;
        if (cur == last) return it;

        size_type index = f(*cur);
        it.node_ = it.node_->children_[index];
        it.positions_.push(index);
        return find_key(std::move(it), std::move(end), f, ++cur, last);
    }

    static void count_keys(const trie_node* root, size_type& count) {
        for (auto& child : root->children_) {
            if (child != nullptr) {
                count_keys(child, count);
                if (child->handle_ != nullptr) count += 1;
            }
        }
    }
};

} // namespace moat
