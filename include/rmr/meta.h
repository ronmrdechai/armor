// Armor
//
// Copyright Ron Mordechai, 2018
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>

namespace rmr {

namespace detail {

template <typename... Ts>
struct make_void { using type = void; };

} // namespace detail

template <typename... Ts>
using void_t = typename detail::make_void<Ts...>::type;

struct nonesuch {
    nonesuch() = delete;
    ~nonesuch() = delete;
    nonesuch(nonesuch const&) = delete;
    void operator=(nonesuch const&) = delete;
};

namespace detail {

template <typename Default, typename AlwaysVoid, template <typename...> typename Op, typename... Args>
struct detector {
  using value_t = std::false_type;
  using type = Default;
};
 
template <typename Default, template <typename...> typename Op, typename... Args>
struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
  using value_t = std::true_type;
  using type = Op<Args...>;
};

} // namespace detail

template <template <typename...> typename Op, typename... Args>
using is_detected = typename detail::detector<nonesuch, void, Op, Args...>::value_t;
 
template <template <typename...> typename Op, typename... Args>
using detected_t = typename detail::detector<nonesuch, void, Op, Args...>::type;
 
template <typename Default, template <typename...> typename Op, typename... Args>
using detected_or = detail::detector<Default, void, Op, Args...>;

template <template <typename...> typename Op, typename... Args >
constexpr bool is_detected_v = is_detected<Op, Args...>::value;

template <typename Default, template <typename...> typename Op, typename... Args >
using detected_or_t = typename detected_or<Default, Op, Args...>::type;

template <typename Expected, template <typename...> typename Op, typename... Args>
using is_detected_exact = std::is_same<Expected, detected_t<Op, Args...>>;

template <typename Expected, template <typename...> typename Op, typename... Args>
constexpr bool is_detected_exact_v = is_detected_exact<Expected, Op, Args...>::value;

template <typename To, template <typename...> typename Op, typename... Args>
using is_detected_convertible = std::is_convertible<detected_t<Op, Args...>, To>;

template <typename To, template <typename...> typename Op, typename... Args>
constexpr bool is_detected_convertible_v = is_detected_convertible<To, Op, Args...>::value;

} // namespace rmr
