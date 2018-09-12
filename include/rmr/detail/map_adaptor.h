// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

namespace rmr::detail {

template <typename T, typename Trie>
class map_adaptor {
public:
    using key_type               = typename Trie::key_type;
    using mapped_type            = T;
    using char_type              = typename Trie::char_type;
    using value_type             = typename Trie::value_type;
    using size_type              = typename Trie::size_type;
    using difference_type        = typename Trie::difference_type;
    using key_mapper             = typename Trie::key_mapper;
    using allocator_type         = typename Trie::allocator_type;
    using reference              = typename Trie::reference;
    using const_reference        = typename Trie::const_reference;
    using pointer                = typename Trie::pointer;
    using const_pointer          = typename Trie::const_pointer;
    using iterator               = typename Trie::iterator;
    using const_iterator         = typename Trie::const_iterator;
    using reverse_iterator       = typename Trie::reverse_iterator;
    using const_reverse_iterator = typename Trie::const_reverse_iterator;

    map_adaptor() = default;

    explicit map_adaptor(key_mapper km, allocator_type alloc = allocator_type()) :
        trie_(std::move(km), std::move(alloc)) {}
    explicit map_adaptor(allocator_type alloc) : trie_(std::move(alloc)) {}

    template <typename InputIterator>
    map_adaptor(InputIterator first, InputIterator last,
         key_mapper km = key_mapper(), allocator_type alloc = allocator_type()
    ) : map_adaptor(std::move(km), std::move(alloc)) { insert(first, last); }
    template <typename InputIterator>
    map_adaptor(InputIterator first, InputIterator last, allocator_type alloc) :
        map_adaptor(std::move(alloc)) { insert(first, last); }

    map_adaptor(const map_adaptor&) = default;
    map_adaptor(const map_adaptor& other, allocator_type alloc) : trie_(other, std::move(alloc)) {}

    map_adaptor(map_adaptor&& other) = default;
    map_adaptor(map_adaptor&& other, allocator_type alloc) : trie_(std::move(other), std::move(alloc)) {}

    map_adaptor(std::initializer_list<value_type> init,
        key_mapper km = key_mapper(),
        allocator_type alloc = allocator_type()
    ) : map_adaptor(init.begin(), init.end(), std::move(km), std::move(alloc)) {}
    map_adaptor(std::initializer_list<value_type> init, allocator_type alloc) :
        map_adaptor(init.begin(), init.end(), std::move(alloc)) {}

    map_adaptor& operator=(const map_adaptor&) = default;
    map_adaptor& operator=(map_adaptor&&) noexcept(
        std::allocator_traits<allocator_type>::is_always_equal::value
        && std::is_nothrow_move_assignable<key_mapper>::value
    ) = default;

    map_adaptor& operator=(std::initializer_list<value_type> init) { return *this = map_adaptor(init); }

    allocator_type get_allocator() const { return trie_.get_allocator(); }

    iterator begin() noexcept { return trie_.begin(); }
    const_iterator begin() const noexcept { return trie_.begin(); }
    const_iterator cbegin() const noexcept { return trie_.cbegin(); }

    iterator end() noexcept { return trie_.end(); }
    const_iterator end() const noexcept { return trie_.end(); }
    const_iterator cend() const noexcept { return trie_.cend(); }

    iterator rbegin() noexcept { return trie_.rbegin(); }
    const_iterator rbegin() const noexcept { return trie_.rbegin(); }
    const_iterator crbegin() const noexcept { return trie_.crbegin(); }

    iterator rend() noexcept { return trie_.rend(); }
    const_iterator rend() const noexcept { return trie_.rend(); }
    const_iterator crend() const noexcept { return trie_.crend(); }

    [[nodiscard]] bool empty() const noexcept { return size() == 0; }
    size_type size() const noexcept { return trie_.size(); }
    size_type max_size() const noexcept { return std::numeric_limits<size_type>::max(); }

    void clear() noexcept { trie_.clear(); }

private:
    Trie trie_;
};

} // namespace rmr::detail

#if 0
template <
    typename T,
    std::size_t R,
    typename KeyMapper = identity<std::size_t>,
    typename Key = std::string,
    typename Allocator = std::allocator<std::pair<T, const Key>>
>
class trie_map : public detail::map_adaptor<
    T, detail::trie<typename Allocator::value_type, R, KeyMapper, Key, Allocator>
> {
    static_assert(
        std::is_invocable_r_v<std::size_t, KeyMapper, std::size_t>,
        "KeyMapper is not invocable on std::size_t or does not return std::size_t"
    );
    using base_type = detail::map_adaptor<
        T, detail::trie<typename Allocator::value_type, R, KeyMapper, Key, Allocator>
    >;
public:
    using base_type::base_type;
};
#endif
