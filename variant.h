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

  constexpr variant() noexcept(std::is_nothrow_default_constructible_v<get_nth_type_t<0, Ts...>>) = default;
  // TODO: noexcept
  constexpr variant(variant const&) = default;
  constexpr variant(variant&&) noexcept((std::is_nothrow_move_constructible_v<Ts> && ...)) = default;

  constexpr variant& operator=(variant const&) = default;

  void swap(variant& other) noexcept(((std::is_nothrow_move_constructible_v<Ts> &&
      std::is_nothrow_swappable_v<Ts>) && ...)) {
    if (this->valueless_by_exception() && other.valueless_by_exception()) {
      return;
    }
    if (this->index() == other.index()) {
      using std::swap;
      //swap(gt
    }
  }

  template<typename U, typename... Args, std::enable_if_t<
      cnt_type_v<U, Ts...> == 1 && std::is_constructible_v<U, Args...>, int> = 0>
  constexpr explicit variant(in_place_type_t<U> in_place_flag, Args&& ...args)
      : storage(in_place_flag, std::forward<Args>(args)...) {} // TODO: enable_if for unique U

  template<size_t I, typename... Args, std::enable_if_t<
      I < sizeof...(Ts) && std::is_constructible_v<get_nth_type_t<I, Ts...>, Args...>, int> = 0>
  constexpr explicit variant(in_place_index_t<I> in_place_flag, Args&& ...args)
      : storage(in_place_flag, std::forward<Args>(args)...) {
  } // TODO: enable_if for I < sizeof...(Ts) and constructible

  template<typename T, std::enable_if_t<
      (sizeof...(Ts) > 0)
          && !std::is_same_v<std::decay_t<T>, variant>
          && !is_specialization<T, in_place_type_t>::value
          && !is_size_spec<T, in_place_index_t>::value
          && exactly_once<find_overload_t<T, Ts...>, Ts...>::value
          && std::is_constructible_v<find_overload_t<T, Ts...>, T>, int> = 0>
  constexpr variant(T&& t) noexcept(std::is_nothrow_constructible_v<find_overload_t<T, Ts...>, T>)
      : storage(in_place_type_t<find_overload_t<T, Ts...>>(), std::forward<T>(t)) {}

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

  //  template<typename Target, typename... Types>
  //  friend constexpr Target& get(variant<Types...>& v);

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
  // TODO: unique T
  return get_stg<Target>(v.storage.storage);
}

template<typename Target, typename... Types>
constexpr std::enable_if_t<cnt_type_v<Target, Types...> == 1, Target> const& get(variant<Types...> const& v) {
  if (get_index_of_type_v<Target, Types...> != v.index()) {
    throw bad_variant_access();
  }
// TODO: unique T
  return get_stg<Target>(v.storage.storage);
}

// GET IF
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
  // TODO: unique T
  return &get_stg<Target>(v->storage.storage);
}

template<typename Target, typename... Types>
constexpr std::enable_if_t<cnt_type_v<Target, Types...> == 1, Target> const* get_if(variant<Types...> const* v) {
  if (get_index_of_type_v<Target, Types...> != v->index()) {
    return nullptr;
  }
// TODO: unique T
  return &get_stg<Target>(v->storage.storage);
}

template<typename Target, typename... Types>
constexpr std::enable_if_t<cnt_type_v<Target, Types...> == 1, bool> holds_alternative(variant<Types...> const& v) {
  // TODO: unique T
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
