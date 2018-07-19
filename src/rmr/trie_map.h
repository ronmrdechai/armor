// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <iterator>
#include <initializer_list>
#include <type_traits>
#include <stdexcept>
#include <string>
#include <stack>

#include <rmr/functors.h>

namespace rmr {

namespace detail {

template <typename...> struct rebind;
template <template <typename> typename Type, typename Inner, typename New>
struct rebind<Type<Inner>, New> {
    using type = Type<New>;
};

} // namespace detail

template <
    typename T,
    std::size_t R,
    typename KeyMapper = identity<std::size_t>,
    typename Key = std::string,
    typename Allocator = std::allocator<std::pair<const Key, T>>
>
class trie_map {
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

public:
    class node_type;
private:
    using node_allocator_type = typename detail::rebind<allocator_type, node_type>::type;
    using node_allocator_traits = std::allocator_traits<node_allocator_type>;
public:
    class node_type {
    public:
        using key_type       = trie_map::key_type;
        using mapped_type    = trie_map::mapped_type;
        using value_type     = trie_map::value_type;
        using allocator_type = trie_map::allocator_type;

        constexpr node_type() : value_(nullptr) {}
        constexpr node_type(allocator_type allocator) :
            value_(nullptr), allocator_(std::move(allocator))
        {} 
        node_type(node_type&&) = default;
        node_type& operator=(node_type&&) = default;

        bool             empty() const { return value_ == nullptr; }
        explicit operator bool() const { return !empty(); }

        key_type&       key()    const { return const_cast<key_type&>(value_->first); }
        mapped_type&    mapped() const { return value_->second; }
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
        std::optional<node_allocator_type> allocator_;

        friend class trie_map;
    };

    struct link_type;
    using link_allocator_type = typename detail::rebind<allocator_type, link_type>::type;
    using link_allocator_traits = std::allocator_traits<link_allocator_type>;
    struct link_type {
        std::array<link_type*, R> children = { nullptr };
        link_type*                parent   = nullptr;
        node_type*                handle   = nullptr;

        void destroy(
            node_allocator_type& node_allocator, link_allocator_type& link_allocator
        ) {
            if (handle != nullptr) {
                node_allocator_traits::deallocate(node_allocator, handle, 1);
                handle = nullptr;
            }

            for (auto& child : children) {
                if (child != nullptr) {
                    child->destroy(node_allocator, link_allocator);
                    link_allocator_traits::deallocate(link_allocator, child, 1);
                    child = nullptr;
                }
            }
        }
    };

public:
    trie_map() : size_(0) { init(); }
    explicit trie_map(key_mapper f, allocator_type allocator = allocator_type()) :
        size_(0), key_map_(std::move(f)), allocator_(std::move(allocator))
    { init(); }
    explicit trie_map(allocator_type allocator) :
        size_(0), allocator_(std::move(allocator)) 
    { init(); }

    trie_map(trie_map&&) = default;
    trie_map(trie_map&& other, allocator_type allocator) :
        base_(std::move(other.base_)),
        root_(std::move(other.root_)),
        size_(0),
        allocator_(std::move(allocator)),
        key_map_(std::move(other.key_map_))
    {}

    template <typename InputIt>
    trie_map(
        InputIt first,
        InputIt last,
        key_mapper f = key_mapper(),
        allocator_type allocator = allocator_type()
    ) : size_(0), allocator_(std::move(allocator)), key_map_(std::move(f)) {
        init();
        for (auto cur = first; cur != last; ++cur) insert(*cur);
    }

    trie_map(
        std::initializer_list<value_type> init,
        key_mapper f = key_mapper(),
        allocator_type allocator = allocator_type()
    ) : trie_map(init.begin(), init.end(), std::move(f), std::move(allocator)) {}
    trie_map(std::initializer_list<value_type> init, allocator_type allocator) :
        trie_map(init.begin(), init.end(), key_mapper(), std::move(allocator))
    {}

    trie_map(const trie_map& other) : trie_map(other.begin(), other.end()) {}
    trie_map(const trie_map& other, allocator_type allocator) :
        trie_map(other.begin(), other.end(), std::move(allocator))
    {}

    ~trie_map() { root_.destroy(node_allocator_, link_allocator_); }

    trie_map& operator=(trie_map&&) = default;
    trie_map& operator=(const trie_map&& other) {
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
        if (it == end()) throw std::out_of_range("rmr::trie_map::at");
        return it->second;
    }
    const mapped_type& at(const key_type& key) const {
        const_iterator it = find(key);
        if (it == end()) throw std::out_of_range("rmr::trie_map::at");
        return it->second;
    }

    template <typename NodeT>
    class generic_iterator {
    public:
        using difference_type   = std::ptrdiff_t;
        using value_type        = typename trie_map::value_type;
        using reference         = typename trie_map::reference;
        using pointer           = typename trie_map::pointer;
        using iterator_category = std::forward_iterator_tag;

        generic_iterator() : link_(nullptr) {}
        generic_iterator(NodeT* node) : link_(std::move(node)) {}
        generic_iterator(const generic_iterator&) = default;
        generic_iterator& operator=(const generic_iterator&) = default;

        template <typename = std::enable_if_t<std::is_const_v<NodeT>>>
        generic_iterator(const generic_iterator<std::remove_const_t<NodeT>>& other) :
            link_(const_cast<const NodeT*>(other.link_)), positions_(other.positions_)
        {}

        generic_iterator& operator++() {
            do {
                *this = next(std::move(*this));
            } while(this->link_->handle == nullptr && !positions_.empty());
            return *this;
        }

        generic_iterator operator++(int) {
            generic_iterator it = next(*this);
            while (it.link_->handle == nullptr && !positions_.empty()) {
                it = next(it);
            }
            return it;
        }

        reference operator*() const {
            return *(link_->handle->value_);
        }

        pointer operator->() const {
            return link_->handle->value_;
        }

        bool operator==(const generic_iterator& other) const {
            return (link_ == other.link_) && (positions_ == other.positions_);
        }

        bool operator!=(const generic_iterator& other) const {
            return !(*this == other);
        }

        friend void swap(generic_iterator& lhs, generic_iterator& rhs) {
            std::swap(lhs.link_, rhs.link_);
            std::swap(lhs.positions_, rhs.positions_);
        }

    private:
        NodeT*                link_;
        std::stack<size_type> positions_;

        static std::pair<generic_iterator, bool>
        step_down(generic_iterator it) {
            for (size_type pos = 0; pos < R; ++pos) {
                if (it.link_->children[pos] != nullptr) {
                    it.link_ = it.link_->children[pos];
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
                link_type* parent = it.link_->parent;
                if (parent->children[pos] != nullptr) {
                    it.link_ = parent->children[pos];
                    it.positions_.top() = pos;
                    return {it, true};
                }
            }
            return {it, false};
        }

        static std::pair<generic_iterator, bool>
        step_up(generic_iterator it) {
            it.link_ = it.link_->parent;
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

        friend class trie_map;
    };

    using iterator = generic_iterator<link_type>;
    using const_iterator = generic_iterator<const link_type>;

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
            root_.children.begin(),
            root_.children.end(),
            [](link_type* child) { return child == nullptr; }
        );
    }

    size_type size() const {
        return size_;
    }

    void clear() {
       root_.destroy(node_allocator_, link_allocator_);
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
            node_allocator_traits::deallocate(node_allocator_, nh, 1);
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

    struct insert_return_type {
        iterator  position;
        bool      inserted;
        node_type node;
    };

    std::pair<iterator, bool> insert(const value_type& value) {
        iterator it = find(value.first);
        if (it != end()) return {it, false};

        insert_handle(&root_, &base_, value.first, 0, make_handle(value));
        return {find(value.first), true};
    }
    std::pair<iterator, bool> insert(value_type&& value) {
        iterator it = find(value.first);
        if (it != end()) return {it, false};

        insert_handle(&root_, &base_, value.first, 0, make_handle(std::move(value)));
        return {find(value.first), true};
    }
    insert_return_type insert(node_type&& nh) {
        insert_return_type ret{};
        if (nh.empty()) {
            ret.position = end();
            ret.inserted = false;
            return ret;
        }
        // ASSERT nh.get_allocator() == node_allocator_

        auto [it, inserted] = insert(nh.value());
        if (inserted) {
            ret.position = it;
            nh.value_ = nullptr;
            ret.inserted = true;
        } else {
            ret.node = std::move(nh);
            ret.position = it;
            ret.inserted = false;
        }
        return ret;
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

    bool operator==(const trie_map& other) const {
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
    bool operator!=(const trie_map& other) const {
        return !(*this == other);
    }

    void swap(trie_map& other) {
        using std::swap;

        swap(base_, other.base_);
        swap(root_, other.root_);
        swap(allocator_, other.allocator_);
        swap(key_map_, other.key_map_);
    }
    friend void swap(trie_map& lhs, trie_map& rhs) {
        lhs.swap(rhs);
    }

    node_type extract(const_iterator pos) {
        node_type handle = std::move(*pos.link_->handle);
        erase(pos);
        return std::move(handle);
    }
    node_type extract(const key_type& key) {
        return extract(const_iterator(find(key)));
    }

    iterator erase(const_iterator pos) {
        iterator it;
        it.link_ = const_cast<link_type*>(pos.link_);
        it.positions_ = pos.positions_;
        return erase(it);
    }

    iterator erase(iterator pos) {
        size_t child = pos.positions_.top();
        iterator ret = std::next(pos);

        while (pos.link_->parent != &base_ && children_count(pos.link_->parent) == 1) {
            pos.link_ = pos.link_->parent;
            pos.positions_.pop();
            child = pos.positions_.top();
        }

        auto& parent = pos.link_->parent;
        pos.link_->destroy(node_allocator_, link_allocator_);
        if (pos.link_ != &root_) {
            parent->children[child] = nullptr;
            link_allocator_traits::deallocate(link_allocator_, pos.link_, 1);
        }

        --size_;
        return ret;
    }
    iterator erase(const_iterator first, const_iterator last) {
        for (auto it = first; it != last; ++it) erase(it);
        iterator ret(const_cast<link_type*>(last.link_));
        ret.positions_ = last.positions_;
        return ret;
    }
    size_type erase(const key_type& key) {
        iterator it = find(key);
        if (it == end()) return 0;
        erase(it);
        return 1;
    }

    allocator_type get_allocator() const { return allocator_; }
    key_mapper     key_map()       const { return key_map_; }

private:
    link_type            base_;
    link_type            root_;
    size_type            size_;
    allocator_type       allocator_;
    link_allocator_type  link_allocator_;
    node_allocator_type  node_allocator_;
    key_mapper           key_map_;

    void init() {
        base_.children[0] = &root_;
        root_.parent = &base_;
    }

    template <typename... Args>
    node_type* make_handle(Args&&... args) {
        node_type* handle = node_allocator_traits::allocate(node_allocator_, 1);
        node_allocator_traits::construct(node_allocator_, handle, node_allocator_);
        handle->value_ = allocator_traits::allocate(allocator_, 1);
        allocator_traits::construct(allocator_, handle->value_, std::forward<Args>(args)...);
        return handle;
    }

    link_type* make_link_type(link_type* parent) {
        link_type* node = link_allocator_traits::allocate(link_allocator_, 1);
        link_allocator_traits::construct(link_allocator_, node);
        node->parent = parent;
        return node;
    }

    link_type* insert_handle(
        link_type* root,
        link_type* parent,
        const key_type& key,
        size_type index,
        node_type* nh
    ) {
        if (root == nullptr) root = make_link_type(parent);

        if (index == key.size()) {
            if (root->handle == nullptr) { root->handle = nh; ++size_; }
            else                         node_allocator_traits::deallocate(node_allocator_, nh, 1);
        } else {
            auto& child = root->children[key_map_(key[index])];
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
        It it, It end,
        const key_mapper& f,
        typename key_type::const_iterator cur,
        typename key_type::const_iterator last
    ) {
        if (it.link_ == nullptr) return end;
        if (cur == last) return it;

        size_type index = f(*cur);
        it.link_ = it.link_->children[index];
        it.positions_.push(index);
        return find_key(std::move(it), std::move(end), f, ++cur, last);
    }

    static size_t children_count(const link_type* root) {
        size_t count = 0;
        for (auto& child : root->children) {
            count += (child != nullptr);
        }
        return count;
    }
};

} // namespace rmr
