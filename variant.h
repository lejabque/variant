#pragma once

#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "variant_storage.h"

struct bad_variant_access : std::exception {
  const char* what() const noexcept override { return "bad_variant_access"; }
};

template<typename... Ts>
struct variant {

  constexpr variant() noexcept(variant_traits<Ts...>::noexcept_value::default_ctor) = default;
  constexpr variant(variant const&) = default;
  constexpr variant(variant&&) noexcept(variant_traits<Ts...>::noexcept_value::move_ctor) = default;

  constexpr variant& operator=(variant const&) = default;
  constexpr variant& operator=(variant&&) noexcept(variant_traits<Ts...>::noexcept_value::move_assign) = default;

  void swap(variant& other) noexcept(variant_traits<Ts...>::noexcept_value::swap) {
    storage.swap(other.storage);
  }

  template<typename U, typename... Args, std::enable_if_t<
      cnt_type_v<U, Ts...> == 1 && std::is_constructible_v<U, Args...>, int> = 0>
  constexpr explicit variant(in_place_type_t<U> in_place_flag, Args&& ...args)
      : storage(in_place_flag, std::forward<Args>(args)...) {}

  template<size_t I, typename... Args, std::enable_if_t<
      I < sizeof...(Ts) && std::is_constructible_v<get_nth_type_t<I, Ts...>, Args...>, int> = 0>
  constexpr explicit variant(in_place_index_t<I> in_place_flag, Args&& ...args)
      : storage(in_place_flag, std::forward<Args>(args)...) {
  }

  template<typename T, std::enable_if_t<
      (sizeof...(Ts) > 0)
          && !std::is_same_v<std::decay_t<T>, variant>
          && !is_specialization<T, in_place_type_t>::value
          && !is_size_spec<T, in_place_index_t>::value
          && exactly_once<find_overload_t<T, Ts...>, Ts...>::value
          && std::is_constructible_v<find_overload_t<T, Ts...>, T>, int> = 0>
  constexpr variant(T&& t) noexcept(std::is_nothrow_constructible_v<find_overload_t<T, Ts...>, T>)
      : storage(in_place_type_t<find_overload_t<T, Ts...>>(), std::forward<T>(t)) {}

  template<typename T, std::enable_if_t<
      (sizeof...(Ts) > 0)
          && !std::is_same_v<std::decay_t<T>, variant>
          && !is_specialization<T, in_place_type_t>::value
          && !is_size_spec<T, in_place_index_t>::value
          && exactly_once<find_overload_t<T, Ts...>, Ts...>::value
          && std::is_constructible_v<find_overload_t<T, Ts...>, T>, int> = 0>
  variant& operator=(T&& t) noexcept(std::is_nothrow_constructible_v<find_overload_t<T, Ts...>, T>) {
    using Type = find_overload_t<T, Ts...>;
    if (this->index() == get_index_of_type_v<Type, Ts...>) {
      get<get_index_of_type_v<Type, Ts...>>(*this) = std::forward<T>(t);
    } else {
      if constexpr (std::is_nothrow_constructible_v<Type, T> || !std::is_nothrow_move_constructible_v<Type>) {
        this->template emplace<get_index_of_type_v<Type, Ts...>>(std::forward<T>(t));
      } else {
        this->operator=(variant(std::forward<T>(t)));
      }
    }
    return *this;
  }

  template<size_t I, class... Args>
  get_nth_type_t<I, Ts...>& emplace(Args&& ...args) {
    return storage.template emplace<I>(std::forward<Args>(args)...);
  }

  template<typename T, class... Args>
  T& emplace(Args&& ...args) {
    return storage.template emplace<get_index_of_type_v<T, Ts...>>(std::forward<Args>(args)...);
  }

  constexpr size_t index() const noexcept {
    return storage.index_;
  }

  constexpr bool valueless_by_exception() const noexcept {
    return index() == variant_npos;
  }

  template<size_t ind, typename... Types>
  friend constexpr get_nth_type_t<ind, Types...>& get(variant<Types...>& v);

  template<size_t ind, typename... Types>
  friend constexpr get_nth_type_t<ind, Types...> const& get(variant<Types...> const& v);

  template<typename Target, typename... Types>
  friend constexpr std::enable_if_t<cnt_type_v<Target, Types...> == 1, Target>& get(variant<Types...>& v);

  template<typename Target, typename... Types>
  friend constexpr std::enable_if_t<cnt_type_v<Target, Types...> == 1, Target> const& get(variant<Types...> const& v);

  template<size_t ind, typename... Types>
  friend constexpr get_nth_type_t<ind, Types...>* get_if(variant<Types...>* v);

  template<size_t ind, typename... Types>
  friend constexpr get_nth_type_t<ind, Types...> const* get_if(variant<Types...> const* v);

  template<typename Target, typename... Types>
  friend constexpr std::enable_if_t<cnt_type_v<Target, Types...> == 1, Target>* get_if(variant<Types...>* v);

  template<typename Target, typename... Types>
  friend constexpr std::enable_if_t<cnt_type_v<Target, Types...> == 1,
                                    Target> const* get_if(variant<Types...> const* v);

  template<typename Target, typename... Types>
  friend constexpr std::enable_if_t<cnt_type_v<Target, Types...> == 1, bool>
  holds_alternative(variant<Types...> const& v);

 private:
  variant_storage<Ts...> storage;
};

template<size_t ind, typename... Types>
constexpr get_nth_type_t<ind, Types...>& get(variant<Types...>& v) {
  if (ind != v.index()) {
    throw bad_variant_access();
  }
  return get_stg<ind>(v.storage.storage);
}

template<size_t ind, typename... Types>
constexpr get_nth_type_t<ind, Types...> const& get(variant<Types...> const& v) {
  if (ind != v.index()) {
    throw bad_variant_access();
  }
  return get_stg<ind>(v.storage.storage);
}

template<typename Target, typename... Types>
constexpr std::enable_if_t<cnt_type_v<Target, Types...> == 1, Target>& get(variant<Types...>& v) {
  if (get_index_of_type_v<Target, Types...> != v.index()) {
    throw bad_variant_access();
  }
  return get_stg<Target>(v.storage.storage);
}

template<typename Target, typename... Types>
constexpr std::enable_if_t<cnt_type_v<Target, Types...> == 1, Target> const& get(variant<Types...> const& v) {
  if (get_index_of_type_v<Target, Types...> != v.index()) {
    throw bad_variant_access();
  }
  return get_stg<Target>(v.storage.storage);
}

template<size_t ind, typename... Types>
constexpr get_nth_type_t<ind, Types...>* get_if(variant<Types...>* v) {
  if (ind != v->index()) {
    return nullptr;
  }
  return &get_stg<ind>(v->storage.storage);
}

template<size_t ind, typename... Types>
constexpr get_nth_type_t<ind, Types...> const* get_if(variant<Types...> const* v) {
  if (ind != v->index()) {
    return nullptr;
  }
  return &get_stg<ind>(v->storage.storage);
}

template<typename Target, typename... Types>
constexpr std::enable_if_t<cnt_type_v<Target, Types...> == 1, Target>* get_if(variant<Types...>* v) {
  if (get_index_of_type_v<Target, Types...> != v->index()) {
    return nullptr;
  }
  return &get_stg<Target>(v->storage.storage);
}

template<typename Target, typename... Types>
constexpr std::enable_if_t<cnt_type_v<Target, Types...> == 1, Target> const* get_if(variant<Types...> const* v) {
  if (get_index_of_type_v<Target, Types...> != v->index()) {
    return nullptr;
  }
  return &get_stg<Target>(v->storage.storage);
}

template<typename Target, typename... Types>
constexpr std::enable_if_t<cnt_type_v<Target, Types...> == 1, bool> holds_alternative(variant<Types...> const& v) {
  return get_index_of_type_v<Target, Types...> == v.index();
}

template<class T>
struct variant_size;

template<class... Types>
struct variant_size<variant<Types...>> : std::integral_constant<size_t, sizeof...(Types)> {};

template<class... Types>
struct variant_size<const variant<Types...>> : std::integral_constant<size_t, sizeof...(Types)> {};

template<class T> inline constexpr size_t variant_size_v = variant_size<T>::value;

template<size_t I, class T>
struct variant_alternative;

template<size_t I, typename... Ts>
struct variant_alternative<I, variant<Ts...>> {
  using type = get_nth_type_t<I, Ts...>;
};

template<size_t I, typename... Ts>
struct variant_alternative<I, const variant<Ts...>> {
  using type = const get_nth_type_t<I, Ts...>;
};

template<size_t I, class T>
using variant_alternative_t = typename variant_alternative<I, T>::type;

template<class T>
struct variant_indexes;

template<typename... Ts>
struct variant_indexes<variant<Ts...>> {
  using type = std::index_sequence_for<Ts...>;
};

template<typename... Ts>
struct variant_indexes<const variant<Ts...>> {
  using type = std::index_sequence_for<Ts...>;
};

template<class T>
using variant_indexes_t = typename variant_indexes<T>::type;

template<typename Visitor, typename... Variants>
struct table_impl_last {
  template<size_t... Is>
  static constexpr auto get_func(std::index_sequence<Is...>) {
    return [](Visitor&& vis, Variants... vars) {
      return vis(get<Is>(std::forward<Variants>(vars))...);
    };
  }
};

template<bool is_last, typename R, typename Visitor, size_t Current, typename... Variants>
struct table_impl {
  template<size_t... Prefix, size_t... VariantIndexes>
  static constexpr auto make_table(std::index_sequence<Prefix...>, std::index_sequence<VariantIndexes...>) {
    return std::experimental::make_array(
        table_impl<Current + 2 == sizeof...(Variants),
                   R,
                   Visitor,
                   Current + 1,
                   Variants...>::make_table(std::index_sequence<Prefix..., VariantIndexes>{},
                                            variant_indexes_t<std::decay_t<get_nth_type_t<
                                                Current + 1,
                                                Variants...>>>{})...
    );
  }
};

template<typename R, typename Visitor, size_t Current, typename... Variants>
struct table_impl<true, R, Visitor, Current, Variants...> {
  template<size_t... Prefix, size_t... VariantIndexes>
  static constexpr auto make_table(std::index_sequence<Prefix...>, std::index_sequence<VariantIndexes...>) {
    return std::experimental::make_array(
        table_impl_last<Visitor, Variants...>::get_func(std::index_sequence<Prefix..., VariantIndexes>{})...
    ); // TODO: закешировать в static
  }
};

template<typename Visitor, typename... Variants>
constexpr decltype(auto) visit(Visitor&& vis, Variants&& ... vars) {
  if ((vars.valueless_by_exception() || ...)) {
    throw bad_variant_access();
  }
  return get_from_table(table_impl<sizeof...(Variants) == 1,
                                   std::invoke_result_t<Visitor, variant_alternative_t<0, std::decay_t<Variants>>...>,
                                   Visitor, 0,
                                   Variants&& ...>::make_table(std::index_sequence<>{},
                                                               variant_indexes_t<std::decay_t<get_nth_type_t<0,
                                                                                                             Variants...>>>{}),
                        vars...)(std::forward<Visitor>(vis),
                                 std::forward<Variants>(vars)...);
}

