#pragma once

template<size_t ind, bool less, typename... Ts>
struct get_nth_type_impl;

template<size_t ind, typename... Ts>
struct get_nth_type_impl<ind, false, Ts...> {};

template<size_t ind, typename... Ts>
struct get_nth_type_impl<ind, true, Ts...> {
  using type = std::tuple_element_t<ind, std::tuple<Ts...>>;
};

template<size_t ind, typename... Ts>
struct get_nth_type : get_nth_type_impl<ind, ind < sizeof...(Ts), Ts...> {};

template<size_t ind, typename... Ts>
using get_nth_type_t = typename get_nth_type<ind, Ts...>::type;

template<bool is_same, typename Target, typename... Ts>
struct get_index_of_type;

template<typename Target, typename T, typename... Ts>
struct get_index_of_type<false, Target, T, Ts...> {
  static constexpr size_t ind = get_index_of_type<std::is_same_v<T, Target>, Target, Ts...>::ind + 1;
};

template<typename Target, typename... Ts>
struct get_index_of_type<true, Target, Ts...> {
  static constexpr size_t ind = 0;
};

template<typename Target, typename T, typename... Ts>
static constexpr size_t get_index_of_type_v = get_index_of_type<std::is_same_v<T, Target>, Target, Ts...>::ind;

template<typename Target, typename... Ts>
struct cnt_type {
  static constexpr size_t cnt = (std::is_same_v<Ts, Target> + ...);
};

template<typename Target, typename... Ts>
static constexpr size_t cnt_type_v = cnt_type<Target, Ts...>::cnt;

template<typename T>
struct Arr { T x[1]; };

template<bool is_bool, typename U, typename... Ts>
struct find_overload_impl;

template<bool is_bool, typename U, typename T, typename... Ts>
struct find_overload_impl<is_bool, U, T, Ts...> : find_overload_impl<is_bool, U, Ts...> {
  using find_overload_impl<is_bool, U, Ts...>::operator();

  template<typename E = T>
  std::enable_if_t<(!std::is_same_v<std::decay_t<T>, bool> || is_bool)
                       && std::is_same_v<void, std::void_t<decltype(Arr<E>{{std::declval<U>()}})>>,
                   E> operator()(T value) {}
};

template<bool is_bool, typename U, typename T>
struct find_overload_impl<is_bool, U, T> {
  template<typename E = T>
  std::enable_if_t<(!std::is_same_v<std::decay_t<T>, bool> || is_bool)
                       && std::is_same_v<void, std::void_t<decltype(Arr<E>{{std::declval<U>()}})>>,
                   E> operator()(T value) {}
};

template<typename U, typename... Types>
using find_overload_t = typename std::invoke_result_t<find_overload_impl<std::is_same_v<std::decay_t<U>, bool>, U,
                                                                         Types...>, U>;

template<class T>
struct in_place_type_t {
  explicit in_place_type_t() = default;
};

template<class T>
inline constexpr in_place_type_t<T> in_place_type{};

template<size_t I>
struct in_place_index_t {
  explicit in_place_index_t() = default;
};

template<size_t I>
inline constexpr in_place_index_t<I> in_place_index{};

template<typename T, typename... Types>
struct exactly_once {
  static constexpr bool value = (std::is_same_v<T, Types> +  ...) == 1;
};

template<class T, template<class...> class Template>
struct is_specialization : std::false_type {};

template<template<class...> class Template, class... Args>
struct is_specialization<Template<Args...>, Template> : std::true_type {};

template<class T, template<size_t...> class Template>
struct is_size_spec : std::false_type {};

template<template<size_t...> class Template, size_t... Args>
struct is_size_spec<Template<Args...>, Template> : std::true_type {};
//static_assert(std::is_same_v<long, find_overload_v<int, float, long, double>>);
