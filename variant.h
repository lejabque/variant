#pragma once

#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "variant_copy_assign.h"

struct bad_variant_access : std::exception {
  const char* what() const noexcept override { return "bad_variant_access"; }
};

template<size_t ind, typename Variant>
constexpr decltype(auto) get(Variant&& v) {
  if (ind != v.index()) {
    throw bad_variant_access();
  }
  return variant_utils::get_impl<ind>(std::forward<Variant>(v));
}

template<typename Target, typename Variant>
constexpr decltype(auto) get(Variant&& v) {
  return get<variant_utils::variant_type_index_v<Target, std::decay_t<Variant>>>(std::forward<Variant>(v));
}

template<typename... Ts>
class variant : variant_utils::variant_copy_assign_base_t<Ts...>,
                variant_utils::enable_bases<Ts...> {
 private:
  using base = variant_utils::variant_copy_assign_base_t<Ts...>;
  using enable_base = variant_utils::enable_bases<Ts...>;
  using traits = variant_utils::variant_traits<Ts...>;
 public:
  using base::emplace;
  constexpr variant() noexcept(traits::noexcept_value::default_ctor) = default;
  constexpr variant(variant const&) = default;
  constexpr variant(variant&&) noexcept(traits::noexcept_value::move_ctor) = default;

  constexpr variant& operator=(variant const&) = default;
  constexpr variant& operator=(variant&&) noexcept(traits::noexcept_value::move_assign) = default;

  template<typename U, typename... Args, std::enable_if_t<
      variant_utils::exactly_once_v<U, Ts...> && std::is_constructible_v<U, Args...>, int> = 0>
  constexpr explicit variant(in_place_type_t<U> in_place_flag, Args&& ...args)
      : base(in_place_flag, std::forward<Args>(args)...), enable_base{} {}

  template<size_t I, typename... Args, std::enable_if_t<
      I < sizeof...(Ts) && std::is_constructible_v<variant_utils::types_at_t<I, Ts...>, Args...>, int> = 0>
  constexpr explicit variant(in_place_index_t<I> in_place_flag, Args&& ...args)
      : base(in_place_flag, std::forward<Args>(args)...), enable_base{} {}

  template<typename T, std::enable_if_t<(sizeof...(Ts) > 0)
                                            && !std::is_same_v<std::decay_t<T>, variant>
                                            && !variant_utils::is_type_spec_v<T, in_place_type_t>
                                            && !variant_utils::is_size_spec_v<T, in_place_index_t>
                                            && std::is_constructible_v<variant_utils::find_overload_t<T, Ts...>, T>,
                                        int> = 0>
  constexpr variant(T&& t) noexcept(std::is_nothrow_constructible_v<variant_utils::find_overload_t<T, Ts...>, T>)
      : base(in_place_type_t<variant_utils::find_overload_t<T, Ts...>>(), std::forward<T>(t)), enable_base{} {}

  template<typename T, std::enable_if_t<(sizeof...(Ts) > 0)
                                            && !std::is_same_v<std::decay_t<T>, variant>
                                            && std::is_assignable_v<variant_utils::find_overload_t<T, Ts...>&, T>
                                            && std::is_constructible_v<variant_utils::find_overload_t<T, Ts...>, T>,
                                        int> = 0>
  variant& operator=(T&& t) noexcept(std::is_nothrow_assignable_v<variant_utils::find_overload_t<T, Ts...>&, T>
      && std::is_nothrow_constructible_v<variant_utils::find_overload_t<T, Ts...>, T>) {
    using Target = variant_utils::find_overload_t<T, Ts...>;
    if (this->index() == variant_utils::type_index_v<Target, Ts...>) {
      get<variant_utils::type_index_v<Target, Ts...>>(*this) = std::forward<T>(t);
    } else {
      if constexpr (std::is_nothrow_constructible_v<Target, T> || !std::is_nothrow_move_constructible_v<Target>) {
        this->template emplace<variant_utils::type_index_v<Target, Ts...>>(std::forward<T>(t));
      } else {
        this->operator=(variant(std::forward<T>(t)));
      }
    }
    return *this;
  }

  void swap(variant& other) noexcept(traits::noexcept_value::swap) {
    if (this->index_ == variant_npos && other.index_ == variant_npos) {
      return;
    } else if (this->index_ == variant_npos) {
      visit_indexed([this](auto& other_value, auto other_index) {
        this->template emplace<other_index>(std::move(other_value));
      }, other);
      other.destroy_stg();
      other.index_ = variant_npos;
    } else if (other.index_ == variant_npos) {
      visit_indexed([&other](auto& this_value, auto this_index) {
        other.template emplace<this_index>(std::move(this_value));
      }, *this);
      this->destroy_stg();
      this->index_ = variant_npos;
    } else {
      visit_indexed([this, &other](auto& this_value, auto& other_value, auto this_index, auto other_index) {
        if constexpr(this_index == other_index) {
          using std::swap;
          swap(this_value, other_value);
        } else {
          auto tmp = std::move(other_value);
          other.template emplace<this_index>(std::move(this_value));
          this->template emplace<other_index>(std::move(tmp));
        }
      }, *this, other);
    }
  }

  constexpr size_t index() const noexcept {
    return base::index();
  }

  constexpr bool valueless_by_exception() const noexcept {
    return index() == variant_npos;
  }

  template<size_t ind, typename Variant>
  friend constexpr decltype(auto) variant_utils::get_impl(Variant&& v);
};

template<size_t ind, typename... Types>
constexpr auto get_if(variant<Types...>* v) noexcept {
  return v == nullptr || ind != v->index() ? nullptr : &get<ind>(*v);
}

template<size_t ind, typename... Types>
constexpr auto get_if(variant<Types...> const* v) noexcept {
  return v == nullptr || ind != v->index() ? nullptr : &get<ind>(*v);
}

template<typename Target, typename... Types>
constexpr auto get_if(variant<Types...>* v) noexcept {
  return v == nullptr || variant_utils::type_index_v<Target, Types...> != v->index() ?
         nullptr : &get<variant_utils::type_index_v<Target, Types...>>(*v);
}

template<typename Target, typename... Types>
constexpr auto get_if(variant<Types...> const* v) noexcept {
  return v == nullptr || variant_utils::type_index_v<Target, Types...> != v->index() ?
         nullptr : &get<variant_utils::type_index_v<Target, Types...>>(*v);
}

template<typename Target, typename... Types>
constexpr bool holds_alternative(variant<Types...> const& v) {
  return variant_utils::type_index_v<Target, Types...> == v.index();
}

template<typename T>
struct variant_size;

template<typename... Ts>
struct variant_size<variant<Ts...>> : std::integral_constant<size_t, sizeof...(Ts)> {};

template<typename... Ts>
struct variant_size<const variant<Ts...>> : std::integral_constant<size_t, sizeof...(Ts)> {};

template<typename T>
inline constexpr size_t variant_size_v = variant_size<T>::value;

template<size_t I, typename T>
struct variant_alternative;

template<size_t I, typename... Ts>
struct variant_alternative<I, variant<Ts...>> {
  using type = variant_utils::types_at_t<I, Ts...>;
};

template<size_t I, typename... Ts>
struct variant_alternative<I, const variant<Ts...>> {
  using type = const variant_utils::types_at_t<I, Ts...>;
};

template<size_t I, typename T>
using variant_alternative_t = typename variant_alternative<I, T>::type;

template<typename Visitor, typename... Variants>
constexpr decltype(auto) visit(Visitor&& vis, Variants&& ... vars) {
  if ((vars.valueless_by_exception() || ...)) {
    throw bad_variant_access();
  }
  return variant_utils::get_from_table(variant_utils::table_cache<false, Visitor&&, Variants&&...>::array,
                                       vars.index()...)(std::forward<Visitor>(vis),
                                                        std::forward<Variants>(vars)...);
}

template<typename... Ts>
constexpr bool operator==(const variant<Ts...>& v, const variant<Ts...>& w) {
  if (v.index() != w.index()) {
    return false;
  }
  if (v.valueless_by_exception()) {
    return true;
  }
  return visit([](auto&& lhs, auto&& rhs) { return lhs == rhs; }, v, w);
}

template<typename... Ts>
constexpr bool operator!=(const variant<Ts...>& v, const variant<Ts...>& w) {
  if (v.index() != w.index()) {
    return true;
  }
  if (v.valueless_by_exception()) {
    return false;
  }
  return visit([](auto&& lhs, auto&& rhs) { return lhs != rhs; }, v, w);
}

template<typename... Ts>
constexpr bool operator<(const variant<Ts...>& v, const variant<Ts...>& w) {
  if (w.valueless_by_exception()) {
    return false;
  }
  if (v.valueless_by_exception()) {
    return true;
  }
  if (v.index() != w.index()) {
    return v.index() < w.index();
  }
  return visit([](auto&& lhs, auto&& rhs) { return lhs < rhs; }, v, w);
}

template<typename... Ts>
constexpr bool operator>(const variant<Ts...>& v, const variant<Ts...>& w) {
  if (v.valueless_by_exception()) {
    return false;
  }
  if (w.valueless_by_exception()) {
    return true;
  }
  if (v.index() != w.index()) {
    return v.index() > w.index();
  }
  return visit([](auto&& lhs, auto&& rhs) { return lhs > rhs; }, v, w);
}

template<typename... Ts>
constexpr bool operator<=(const variant<Ts...>& v, const variant<Ts...>& w) {
  if (v.valueless_by_exception()) {
    return true;
  }
  if (w.valueless_by_exception()) {
    return false;
  }
  if (v.index() != w.index()) {
    return v.index() < w.index();
  }
  return visit([](auto&& lhs, auto&& rhs) { return lhs <= rhs; }, v, w);
}

template<typename... Ts>
constexpr bool operator>=(const variant<Ts...>& v, const variant<Ts...>& w) {
  if (w.valueless_by_exception()) {
    return true;
  }
  if (v.valueless_by_exception()) {
    return false;
  }
  if (v.index() != w.index()) {
    return v.index() > w.index();
  }
  return visit([](auto&& lhs, auto&& rhs) { return lhs >= rhs; }, v, w);
}
