// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <algorithm>
#include <initializer_list>
#include <stdexcept>
#include <tuple>
#include <utility>

#include <rmr/detail/set_adaptor.h>
#include <rmr/node_handle.h>

namespace rmr::detail {

template <typename T, typename Trie>
class map_adaptor : public set_adaptor<T, Trie> {
    using base_type = set_adaptor<T, Trie>;
public:
    using mapped_type    = T;
    using key_type       = typename base_type::key_type;
    using iterator       = typename base_type::iterator;
    using const_iterator = typename base_type::const_iterator;

    using base_type::base_type;

    mapped_type& at(const key_type& k) {
        iterator it = base_type::find(k);
        if (it == base_type::end()) throw std::out_of_range("rmr::at");
        return it->second;
    }
    const mapped_type& at(const key_type& k) const {
        const_iterator it = base_type::find(k);
        if (it == base_type::end()) throw std::out_of_range("rmr::at");
        return it->second;
    }

    mapped_type& operator[](const key_type& k) { return try_emplace(k).first->second; }
    mapped_type& operator[](key_type&& k) { return try_emplace(std::move(k)).first->second; }

    template <typename M>
    std::pair<iterator, bool> insert_or_assign(const key_type& k, M&& obj) {
        iterator it = base_type::find(k);
        if (it == base_type::end()) return base_type::insert({ k, std::forward<M>(obj) });

        it->second = std::forward<M>(obj);
        return { it, false };
    }
    template <typename M>
    std::pair<iterator, bool> insert_or_assign(key_type&& k, M&& obj) {
        iterator it = base_type::find(k);
        if (it == base_type::end()) return base_type::insert({ std::move(k), std::forward<M>(obj) });

        it->second = std::forward<M>(obj);
        return { it, false };
    }

    template <typename M>
    iterator insert_or_assign(const_iterator hint, const key_type& k, M&& obj) {
        iterator it = base_type::find(k);
        if (it == base_type::end()) return base_type::insert(hint, { k, std::forward<M>(obj) });

        it->second = std::forward<M>(obj);
        return it;
    }
    template <typename M>
    iterator insert_or_assign(const_iterator hint, key_type&& k, M&& obj) {
        iterator it = base_type::find(k);
        if (it == base_type::end()) return base_type::insert(hint, { std::move(k), std::forward<M>(obj) });

        it->second = std::forward<M>(obj);
        return it;
    }

    template <typename... Args>
    std::pair<iterator, bool> try_emplace(const key_type& k, Args&&... args) {
        iterator it = base_type::find(k);
        if (it == base_type::end()) return base_type::emplace(std::piecewise_construct,
            std::forward_as_tuple(k), std::forward_as_tuple(std::forward<Args>(args)...)
        );
        return { it, false };
    }
    template <typename... Args>
    std::pair<iterator, bool> try_emplace(key_type&& k, Args&&... args) {
        iterator it = base_type::find(k);
        if (it == base_type::end()) return base_type::emplace(std::piecewise_construct,
            std::forward_as_tuple(std::move(k)), std::forward_as_tuple(std::forward<Args>(args)...)
        );
        return { it, false };
    }
    template <typename... Args>
    iterator try_emplace(const_iterator hint, const key_type& k, Args&&... args) {
        iterator it = base_type::find(k);
        if (it == base_type::end()) return base_type::emplace_hint(hint, std::piecewise_construct,
            std::forward_as_tuple(k), std::forward_as_tuple(std::forward<Args>(args)...)
        );
        return it;
    }
    template <typename... Args>
    iterator try_emplace(const_iterator hint, key_type&& k, Args&&... args) {
        iterator it = base_type::find(k);
        if (it == base_type::end()) return base_type::emplace_hint(hint, std::piecewise_construct,
            std::forward_as_tuple(std::move(k)), std::forward_as_tuple(std::forward<Args>(args)...)
        );
        return it;
    }

private:
    Trie trie_;
};

} // namespace rmr::detail
