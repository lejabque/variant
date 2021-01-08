#pragma once

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
static constexpr size_t type_index_v = type_index_impl<std::is_same_v<T, Target>, Target, Ts...>::ind;

template<size_t I, class T>
struct variant_alternative;

template<size_t I, template<typename...> class variant, typename... Ts>
struct variant_alternative<I, variant<Ts...>> {
  using type = types_at_t<I, Ts...>;
};

template<size_t I, template<typename...> class variant, typename... Ts>
struct variant_alternative<I, const variant<Ts...>> {
  using type = const types_at_t<I, Ts...>;
};

template<size_t I, template<typename...> class variant, typename... Ts>
struct variant_alternative<I, volatile variant<Ts...>> {
  using type = volatile types_at_t<I, Ts...>;
};

template<size_t I, template<typename...> class variant, typename... Ts>
struct variant_alternative<I, const volatile variant<Ts...>> {
  using type = const volatile types_at_t<I, Ts...>;
};

template<size_t I, class T>
using variant_alternative_t = typename variant_alternative<I, T>::type;

template<typename Target, typename... Ts>
static constexpr size_t cnt_type_v = (std::is_same_v<Ts, Target> + ...);

template<typename T>
struct find_overload_array { T x[1]; };

template<typename U, typename T, typename = void>
struct fun {
  T operator()();
};

template<typename U, typename T>
struct fun<U, T, std::enable_if_t<(!std::is_same_v<std::decay_t<T>, bool> || std::is_same_v<std::decay_t<U>, bool>)
                                           && std::is_same_v<void,
                                                             std::void_t<decltype(find_overload_array<T>{
                                                                 {std::declval<U>()}})>>>> {
  T operator()(T);
};

template<typename U, typename... Ts>
struct find_overload : fun<U, Ts> ... {
  using fun<U, Ts>::operator()...;
};

template<typename U, typename... Ts>
using find_overload_t = typename std::invoke_result_t<find_overload<U, Ts...>, U>;

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
