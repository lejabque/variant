#pragma once
#include "enable_ctor_base.h"
#include "variadic_utils.h"
#include "variant_copy_assign.h"

template<typename... Ts>
struct variant_storage : variant_copy_assign_base_t<Ts...>,
                         enable_bases<Ts...> {
  using base = variant_copy_assign_base_t<Ts...>;
  constexpr variant_storage() noexcept(std::is_nothrow_default_constructible_v<base>) = default;
  void swap(variant_storage& other) noexcept(variant_traits<Ts...>::noexcept_value::swap) {
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
    variant_storage tmp(std::move(other));
    other = std::move(*this);
    *this = std::move(tmp);
  }

  template<typename U, typename... Args>
  constexpr explicit variant_storage(in_place_type_t<U> in_place_flag, Args&& ... args)
      : base(in_place_flag, std::forward<Args>(args)...),
        enable_bases<Ts...>{} {}

  template<size_t I, typename... Args>
  constexpr explicit variant_storage(in_place_index_t<I> in_place_flag, Args&& ... args)
      : base(in_place_flag, std::forward<Args>(args)...),
        enable_bases<Ts...>{} {}

  constexpr variant_storage(variant_storage const&) = default;
  constexpr variant_storage(variant_storage&&) noexcept(std::is_nothrow_move_constructible_v<base>) = default;

  constexpr variant_storage& operator=(variant_storage const&) = default;
  constexpr variant_storage& operator=(variant_storage&&) noexcept(std::is_nothrow_move_assignable_v<base>) = default;

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
    return get_stg<I>(this->storage);
  }
  ~variant_storage() = default;
};
