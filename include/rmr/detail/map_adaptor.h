// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <rmr/detail/adaptor_base.h>

namespace rmr::detail {

template <typename T, typename Trie>
class map_adaptor : public adaptor_base<map_adaptor<T, Trie>, Trie> {
    using base_type = adaptor_base<map_adaptor<T, Trie>, Trie>;
public:
    using key_type               = typename base_type::key_type;
    using mapped_type            = T;
    using char_type              = typename base_type::char_type;
    using value_type             = typename base_type::value_type;
    using size_type              = typename base_type::size_type;
    using difference_type        = typename base_type::difference_type;
    using key_mapper             = typename base_type::key_mapper;
    using allocator_type         = typename base_type::allocator_type;
    using reference              = typename base_type::reference;
    using const_reference        = typename base_type::const_reference;
    using pointer                = typename base_type::pointer;
    using const_pointer          = typename base_type::const_pointer;
    using iterator               = typename base_type::iterator;
    using const_iterator         = typename base_type::const_iterator;
    using reverse_iterator       = typename base_type::reverse_iterator;
    using const_reverse_iterator = typename base_type::const_reverse_iterator;
    using node_type              = typename base_type::node_type;
    using insert_return_type     = typename base_type::insert_return_type;

    using base_type::base_type;

    mapped_type& at(const key_type& k) {
        iterator it = this->find(k);
        if (it == this->end()) throw std::out_of_range("rmr::at");
        return it->second;
    }
    const mapped_type& at(const key_type& k) const {
        const_iterator it = this->find(k);
        if (it == this->end()) throw std::out_of_range("rmr::at");
        return it->second;
    }

    mapped_type& operator[](const key_type& k) { return try_emplace(k).first->second; }
    mapped_type& operator[](key_type&& k) { return try_emplace(std::move(k)).first->second; }

    template <typename M>
    std::pair<iterator, bool> insert_or_assign(const key_type& k, M&& obj) {
        iterator it = this->find(k);
        if (it == this->end()) return this->insert({ k, std::forward<M>(obj) });

        it->second = std::forward<M>(obj);
        return { it, false };
    }
    template <typename M>
    std::pair<iterator, bool> insert_or_assign(key_type&& k, M&& obj) {
        iterator it = this->find(k);
        if (it == this->end()) return this->insert({ std::move(k), std::forward<M>(obj) });

        it->second = std::forward<M>(obj);
        return { it, false };
    }

    template <typename M>
    iterator insert_or_assign(const_iterator hint, const key_type& k, M&& obj) {
        iterator it = this->find(k);
        if (it == this->end()) return this->insert(hint, { k, std::forward<M>(obj) });

        it->second = std::forward<M>(obj);
        return it;
    }
    template <typename M>
    iterator insert_or_assign(const_iterator hint, key_type&& k, M&& obj) {
        iterator it = this->find(k);
        if (it == this->end()) return this->insert(hint, { std::move(k), std::forward<M>(obj) });

        it->second = std::forward<M>(obj);
        return it;
    }

    template <typename... Args>
    iterator emplace_hint(const_iterator hint, Args&&... args ) {
        value_type o(std::forward<Args>(args)...);
        return this->trie_.emplace(hint, o.first, o);
    }
    template <typename... Args>
    std::pair<iterator, bool> try_emplace(const key_type& k, Args&&... args) {
        iterator it = this->find(k);
        if (it == this->end()) return this->emplace(std::piecewise_construct,
            std::forward_as_tuple(k), std::forward_as_tuple(std::forward<Args>(args)...)
        );
        return { it, false };
    }
    template <typename... Args>
    std::pair<iterator, bool> try_emplace(key_type&& k, Args&&... args) {
        iterator it = this->find(k);
        if (it == this->end()) return this->emplace(std::piecewise_construct,
            std::forward_as_tuple(std::move(k)), std::forward_as_tuple(std::forward<Args>(args)...)
        );
        return { it, false };
    }
    template <typename... Args>
    iterator try_emplace(const_iterator hint, const key_type& k, Args&&... args) {
        iterator it = this->find(k);
        if (it == this->end()) return emplace_hint(hint, std::piecewise_construct,
            std::forward_as_tuple(k), std::forward_as_tuple(std::forward<Args>(args)...)
        );
        return it;
    }
    template <typename... Args>
    iterator try_emplace(const_iterator hint, key_type&& k, Args&&... args) {
        iterator it = this->find(k);
        if (it == this->end()) return emplace_hint(hint, std::piecewise_construct,
            std::forward_as_tuple(std::move(k)), std::forward_as_tuple(std::forward<Args>(args)...)
        );
        return it;
    }

    template <typename _Trie> void merge(map_adaptor<T, _Trie>& source) { this->merge_(source); }
    template <typename _Trie> void merge(map_adaptor<T, _Trie>&& source) { merge(source); }
};

} // namespace rmr::detail
