#pragma once

template<typename T>
struct in_place_type_t {
  explicit in_place_type_t() = default;
};

template<typename T>
inline constexpr in_place_type_t<T> in_place_type{};

template<size_t I>
struct in_place_index_t {
  explicit in_place_index_t() = default;
};

template<size_t I>
inline constexpr in_place_index_t<I> in_place_index{};

namespace variant_utils {
template<size_t ind, bool less, typename... Ts>
struct types_at_impl {};

template<size_t ind, typename... Ts>
struct types_at_impl<ind, true, Ts...> {
  using type = std::tuple_element_t<ind, std::tuple<Ts...>>;
};

template<size_t ind, typename... Ts>
using types_at_t = typename types_at_impl<ind, ind < sizeof...(Ts), Ts...>::type;

template<bool is_same, typename Target, typename... Ts>
struct type_index_impl {
  static constexpr size_t ind = 0;
};

template<typename Target, typename T, typename... Ts>
struct type_index_impl<false, Target, T, Ts...> {
  static constexpr size_t ind = type_index_impl<std::is_same_v<T, Target>, Target, Ts...>::ind + 1;
};

template<typename Target, typename T, typename... Ts>
inline constexpr size_t type_index_v = type_index_impl<std::is_same_v<T, Target>, Target, Ts...>::ind;

template<typename T, typename Base>
struct variant_type_index;

template<typename T, template<typename...> typename base, typename... Ts>
struct variant_type_index<T, base<Ts...>> {
  static constexpr size_t value = type_index_v<T, Ts...>;
};

template<typename Target, typename Variant>
inline constexpr size_t variant_type_index_v = variant_type_index<Target, Variant>::value;

template<size_t I, typename T>
struct alternative;

template<size_t I, template<typename...> typename base, typename... Ts>
struct alternative<I, base<Ts...>> {
  using type = types_at_t<I, Ts...>;
};

template<size_t I, template<typename...> typename base, typename... Ts>
struct alternative<I, const base<Ts...>> {
  using type = const types_at_t<I, Ts...>;
};

template<size_t I, template<typename...> typename base, typename... Ts>
struct alternative<I, base<Ts...>&&> {
  using type = types_at_t<I, Ts...>&&;
};

template<size_t I, template<typename...> typename base, typename... Ts>
struct alternative<I, base<Ts...> const&&> {
  using type = const types_at_t<I, Ts...>&&;
};

template<size_t I, template<typename...> typename base, typename... Ts>
struct alternative<I, base<Ts...>&> {
  using type = types_at_t<I, Ts...>&;
};

template<size_t I, template<typename...> typename base, typename... Ts>
struct alternative<I, base<Ts...> const&> {
  using type = types_at_t<I, Ts...> const&;
};

template<size_t I, template<bool, typename...> typename base, bool flag, typename... Ts>
struct alternative<I, base<flag, Ts...>> {
  using type = types_at_t<I, Ts...>;
};

template<size_t I, template<bool, typename...> typename base, bool flag, typename... Ts>
struct alternative<I, base<flag, Ts...> const> {
  using type = const types_at_t<I, Ts...>;
};

template<size_t I, template<bool, typename...> typename base, bool flag, typename... Ts>
struct alternative<I, base<flag, Ts...>&&> {
  using type = types_at_t<I, Ts...>&&;
};

template<size_t I, template<bool, typename...> typename base, bool flag, typename... Ts>
struct alternative<I, base<flag, Ts...> const&&> {
  using type = types_at_t<I, Ts...> const&&;
};

template<size_t I, template<bool, typename...> typename base, bool flag, typename... Ts>
struct alternative<I, base<flag, Ts...>&> {
  using type = types_at_t<I, Ts...>&;
};

template<size_t I, template<bool, typename...> typename base, bool flag, typename... Ts>
struct alternative<I, base<flag, Ts...> const&> {
  using type = types_at_t<I, Ts...> const&;
};

template<size_t I, typename T>
using alternative_t = typename alternative<I, T>::type;

template<class T>
struct alt_indexes;

template<template<typename...> typename base, typename... Ts>
struct alt_indexes<base<Ts...>> {
  using type = std::index_sequence_for<Ts...>;
};

template<template<bool, typename...> typename base, bool flag, typename... Ts>
struct alt_indexes<base<flag, Ts...>> {
  using type = std::index_sequence_for<Ts...>;
};

template<class T>
using alt_indexes_t = typename alt_indexes<std::decay_t<T>>::type;

template<size_t index, bool empty, typename... Ts>
struct alt_indexes_by_ind {
  using type = std::index_sequence<>;
};

template<size_t index, typename... Ts>
struct alt_indexes_by_ind<index, true, Ts...> {
  using type = alt_indexes_t<std::decay_t<types_at_t<index, Ts...>>>;
};

template<size_t index, typename... Ts>
using alt_indexes_by_ind_t = typename alt_indexes_by_ind<index, index < sizeof...(Ts), Ts...>::type;

template<typename T>
struct find_overload_array { T x[1]; };

template<typename U, typename T, size_t ind, typename = void>
struct fun {
  T operator()();
};

template<typename U, typename T, size_t ind>
struct fun<U, T, ind, std::enable_if_t<(!std::is_same_v<std::decay_t<T>, bool> || std::is_same_v<std::decay_t<U>, bool>)
                                           && std::is_same_v<void,
                                                             std::void_t<decltype(find_overload_array<T>{
                                                                 {std::declval<U>()}})>>>> {
  T operator()(T);
};

template<typename U, typename IndexSequence, typename... Ts>
struct find_overload;

// индексы, чтобы различать базы с одинаковыми типами
template<typename U, size_t... Is, typename... Ts>
struct find_overload<U, std::index_sequence<Is...>, Ts...> : fun<U, Ts, Is> ... {
  using fun<U, Ts, Is>::operator()...;
};

template<typename U, typename... Ts>
using find_overload_t = typename std::invoke_result_t<find_overload<U, std::index_sequence_for<Ts...>,
                                                                    Ts...>, U>;

template<typename T, typename... Ts>
inline constexpr bool exactly_once_v = (std::is_same_v<T, Ts> +  ...) == 1;

template<typename T, template<typename...> typename Template>
struct is_type_spec : std::false_type {};

template<template<typename...> typename Template, typename... Args>
struct is_type_spec<Template<Args...>, Template> : std::true_type {};

template<typename T, template<typename...> typename Template>
inline constexpr bool is_type_spec_v = is_type_spec<T, Template>::value;

template<typename T, template<size_t...> typename Template>
struct is_size_spec : std::false_type {};

template<template<size_t...> typename Template, size_t... Args>
struct is_size_spec<Template<Args...>, Template> : std::true_type {};

template<typename T, template<size_t...> typename Template>
inline constexpr bool is_size_spec_v = is_size_spec<T, Template>::value;
} // namespace variadic_utils
