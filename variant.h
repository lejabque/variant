#pragma once

#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "variant_copy_assign.h"

struct bad_variant_access : std::exception {
  const char* what() const noexcept override { return "bad_variant_access"; }
};

template<typename... Ts>
struct variant : variant_copy_assign_base_t<Ts...>,
                 enable_bases<Ts...> {

  using base = variant_copy_assign_base_t<Ts...>;

  constexpr variant() noexcept(variant_traits<Ts...>::noexcept_value::default_ctor) = default;
  constexpr variant(variant const&) = default;
  constexpr variant(variant&&) noexcept(variant_traits<Ts...>::noexcept_value::move_ctor) = default;

  constexpr variant& operator=(variant const&) = default;
  constexpr variant& operator=(variant&&) noexcept(variant_traits<Ts...>::noexcept_value::move_assign) = default;

//  void swap(variant& other) noexcept(variant_traits<Ts...>::noexcept_value::swap) {
//    storage.swap(other.storage);
//  }
  void swap(variant& other) noexcept(variant_traits<Ts...>::noexcept_value::swap) {
    if (this->index_ == variant_npos && other.index_ == variant_npos) {
      return;
    }
    if (this->index_ == other.index_) {
      auto visiter = [](auto&& this_value, auto&& other_value, auto this_index, auto other_index) {
        if constexpr (decltype(this_index)::value == decltype(other_index)::value) {
          using std::swap;
          swap(this_value, other_value);
        }
      };
      visit_indexed(visiter, *this, other);
      return;
    }
    variant tmp(std::move(other));
    other = std::move(*this);
    *this = std::move(tmp);
  }

  template<typename U, typename... Args, std::enable_if_t<
      exactly_once_v<U, Ts...> && std::is_constructible_v<U, Args...>, int> = 0>
  constexpr explicit variant(in_place_type_t<U> in_place_flag, Args&& ...args)
      : base(in_place_flag, std::forward<Args>(args)...),
        enable_bases<Ts...>{} {}

  template<size_t I, typename... Args, std::enable_if_t<
      I < sizeof...(Ts) && std::is_constructible_v<types_at_t<I, Ts...>, Args...>, int> = 0>
  constexpr explicit variant(in_place_index_t<I> in_place_flag, Args&& ...args)
      : base(in_place_flag, std::forward<Args>(args)...), enable_bases<Ts...>{} {}

  template<typename T, std::enable_if_t<
      (sizeof...(Ts) > 0)
          && !std::is_same_v<std::decay_t<T>, variant>
          && !is_specialization<T, in_place_type_t>::value
          && !is_size_spec<T, in_place_index_t>::value
          && exactly_once_v<find_overload_t<T, Ts...>, Ts...>
          && std::is_constructible_v<find_overload_t<T, Ts...>, T>, int> = 0>
  constexpr variant(T&& t) noexcept(std::is_nothrow_constructible_v<find_overload_t<T, Ts...>, T>)
      : base(in_place_type_t<find_overload_t<T, Ts...>>(), std::forward<T>(t)), enable_bases<Ts...>{} {}

  template<typename T, std::enable_if_t<
      (sizeof...(Ts) > 0)
          && !std::is_same_v<std::decay_t<T>, variant>
          && !is_specialization<T, in_place_type_t>::value
          && !is_size_spec<T, in_place_index_t>::value
          && exactly_once_v<find_overload_t<T, Ts...>, Ts...>
          && std::is_constructible_v<find_overload_t<T, Ts...>, T>, int> = 0>
  variant& operator=(T&& t) noexcept(std::is_nothrow_constructible_v<find_overload_t<T, Ts...>, T>) {
    using Type = find_overload_t<T, Ts...>;
    if (this->index() == type_index_v<Type, Ts...>) {
      get<type_index_v<Type, Ts...>>(*this) = std::forward<T>(t);
    } else {
      if constexpr (std::is_nothrow_constructible_v<Type, T> || !std::is_nothrow_move_constructible_v<Type>) {
        this->template emplace<type_index_v<Type, Ts...>>(std::forward<T>(t));
      } else {
        this->operator=(variant(std::forward<T>(t)));
      }
    }
    return *this;
  }

  template<size_t I, class... Args>
  types_at_t<I, Ts...>& emplace(Args&& ...args) {
    if (this->index_ != variant_npos) {
      this->destroy_stg();
    }
    try {
      this->storage.template emplace_stg<I>(std::forward<Args>(args)...);
    } catch (...) {
      this->index_ = variant_npos;
      throw;
    }
    this->index_ = I;
    return this->storage.template get_stg<I>();
    // return this->.template emplace<I>(std::forward<Args>(args)...);
  }

  template<typename T, class... Args>
  T& emplace(Args&& ...args) {
    return this->template emplace<type_index_v<T, Ts...>>(std::forward<Args>(args)...);
  }

  constexpr size_t index() const noexcept {
    return this->index_;
  }

  constexpr bool valueless_by_exception() const noexcept {
    return index() == variant_npos;
  }

  template<size_t ind, typename... Types>
  friend constexpr types_at_t<ind, Types...>& get(variant<Types...>& v);

  template<size_t ind, typename... Types>
  friend constexpr types_at_t<ind, Types...> const& get(variant<Types...> const& v);

  template<typename Target, typename... Types>
  friend constexpr std::enable_if_t<exactly_once_v<Target, Types...>, Target>& get(variant<Types...>& v);

  template<typename Target, typename... Types>
  friend constexpr std::enable_if_t<exactly_once_v<Target, Types...>, Target> const& get(variant<Types...> const& v);

  template<size_t ind, typename... Types>
  friend constexpr types_at_t<ind, Types...>* get_if(variant<Types...>* v);

  template<size_t ind, typename... Types>
  friend constexpr types_at_t<ind, Types...> const* get_if(variant<Types...> const* v);

  template<typename Target, typename... Types>
  friend constexpr std::enable_if_t<exactly_once_v<Target, Types...>, Target>* get_if(variant<Types...>* v);

  template<typename Target, typename... Types>
  friend constexpr std::enable_if_t<exactly_once_v<Target, Types...>,
                                    Target> const* get_if(variant<Types...> const* v);

  template<typename Target, typename... Types>
  friend constexpr std::enable_if_t<exactly_once_v<Target, Types...>,
                                    bool> holds_alternative(variant<Types...> const& v);

// private:
//  variant_storage<Ts...> storage;
};

template<size_t ind, typename... Types>
constexpr types_at_t<ind, Types...>& get(variant<Types...>& v) {
  if (ind != v.index()) {
    throw bad_variant_access();
  }
  return v.storage.template get_stg<ind>();
}

template<size_t ind, typename... Types>
constexpr types_at_t<ind, Types...> const& get(variant<Types...> const& v) {
  if (ind != v.index()) {
    throw bad_variant_access();
  }
  return v.storage.template get_stg<ind>();
}

template<typename Target, typename... Types>
constexpr std::enable_if_t<exactly_once_v<Target, Types...>, Target>& get(variant<Types...>& v) {
  return get<type_index_v<Target, Types...>>(v);
}

template<typename Target, typename... Types>
constexpr std::enable_if_t<exactly_once_v<Target, Types...>, Target> const& get(variant<Types...> const& v) {
  return get<type_index_v<Target, Types...>>(v);
}

template<size_t ind, typename... Types>
constexpr types_at_t<ind, Types...>* get_if(variant<Types...>* v) {
  if (ind != v->index()) {
    return nullptr;
  }
  return &v->storage.template get_stg<ind>();
//  return &get_stg<ind>(v->storage.storage);
}

template<size_t ind, typename... Types>
constexpr types_at_t<ind, Types...> const* get_if(variant<Types...> const* v) {
  if (ind != v->index()) {
    return nullptr;
  }
  return &v->storage.template get_stg<ind>();
}

template<typename Target, typename... Types>
constexpr std::enable_if_t<exactly_once_v<Target, Types...>, Target>* get_if(variant<Types...>* v) {
  return v == nullptr ? nullptr : &get<type_index_v<Target, Types...>>(*v);
}

template<typename Target, typename... Types>
constexpr std::enable_if_t<exactly_once_v<Target, Types...>, Target> const* get_if(variant<Types...> const* v) {
  return v == nullptr ? nullptr : &get<type_index_v<Target, Types...>>(*v);
}

template<typename Target, typename... Types>
constexpr std::enable_if_t<exactly_once_v<Target, Types...>, bool> holds_alternative(variant<Types...> const& v) {
  return type_index_v<Target, Types...> == v.index();
}

template<class T>
struct variant_size;

template<class... Types>
struct variant_size<variant<Types...>> : std::integral_constant<size_t, sizeof...(Types)> {};

template<class... Types>
struct variant_size<const variant<Types...>> : std::integral_constant<size_t, sizeof...(Types)> {};

template<class T> inline constexpr size_t variant_size_v = variant_size<T>::value;

template<size_t I, typename T>
struct variant_alternative;

template<size_t I, template<typename...> typename variant, typename... Ts>
struct variant_alternative<I, variant<Ts...>> {
  using type = types_at_t<I, Ts...>;
};

template<size_t I, template<typename...> typename variant, typename... Ts>
struct variant_alternative<I, const variant<Ts...>> {
  using type = const types_at_t<I, Ts...>;
};

template<size_t I, template<typename...> typename variant, typename... Ts>
struct variant_alternative<I, volatile variant<Ts...>> {
  using type = volatile types_at_t<I, Ts...>;
};

template<size_t I, template<typename...> typename variant, typename... Ts>
struct variant_alternative<I, const volatile variant<Ts...>> {
  using type = const volatile types_at_t<I, Ts...>;
};

template<size_t I, typename T>
using variant_alternative_t = typename variant_alternative<I, T>::type;

template<typename Visitor, typename... Variants>
constexpr decltype(auto) visit(Visitor&& vis, Variants&& ... vars) {
  if ((vars.valueless_by_exception() || ...)) {
    throw bad_variant_access();
  }
  return get_from_table(table_cache_t<false,
                                      std::invoke_result_t<Visitor,
                                                           alternative_t<0, Variants>...>,
                                      Visitor&&,
                                      Variants&& ...>::array, vars.index()...)(std::forward<Visitor>(vis),
                                                                               std::forward<Variants>(vars)...);
}
