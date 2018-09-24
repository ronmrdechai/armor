// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <rmr/detail/adaptor_base.h>

namespace rmr::detail {

template <typename Trie>
class set_adaptor : public adaptor_base<set_adaptor<Trie>, Trie> {
    using base_type = adaptor_base<set_adaptor<Trie>, Trie>;
public:
    using key_type               = typename base_type::key_type;
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

    template <typename... Args>
    iterator emplace_hint(const_iterator hint, Args&&... args ) {
        value_type o(std::forward<Args>(args)...);
        return this->trie_.emplace(hint, o, o);
    }

    template <typename _Trie>
    void merge(set_adaptor<_Trie>& source) {
        for (auto it = source.begin(), last = source.end(); it != last;) {
            auto pos = it++;
            if (this->find(*pos) != this->end()) continue;
            this->insert(source.extract(pos));
        }
    }
    template <typename _Trie> void merge(set_adaptor<_Trie>&& source) { merge(source); }
};

} // namespace rmr::detail
