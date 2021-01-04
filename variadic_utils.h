#pragma once

struct non_existing_type {
  non_existing_type() = delete;
};

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

template<typename Target, typename T, typename... Ts>
struct cnt_type {
  static constexpr size_t cnt = cnt_type<Target, Ts...>::cnt + std::is_same_v<T, Target>;
};

template<typename Target, typename T>
struct cnt_type<Target, T> {
  static constexpr size_t cnt = std::is_same_v<T, Target>;
};

template<typename Target, typename... Ts>
static constexpr size_t cnt_type_v = cnt_type<Target, Ts...>::cnt;

// https://en.cppreference.com/w/cpp/types/type_identity since C++20 😢
template<class T>
struct type_identity {
  using type = T;
};

template<typename... Types>
struct find_overload;

template<typename T, typename... Types>
struct find_overload<T, Types...> : find_overload<Types...> {
  using find_overload<Types...>::operator();

  type_identity<T> operator()(T value) {}
};

template<typename T>
struct find_overload<T> {
  type_identity<T> operator()(T value) {}
};

template<typename U, typename... Types>
using find_overload_v = typename std::invoke_result_t<find_overload<Types...>, U>::type;

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