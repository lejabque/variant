#pragma once
#include "variant_dtor.h"

template<bool is_trivial, typename... Ts>
struct variant_move_ctor_base : variant_dtor_base_t<Ts...> {
  using base = variant_dtor_base_t<Ts...>;
  using base::base;
  constexpr variant_move_ctor_base() noexcept(std::is_nothrow_default_constructible_v<base>) = default;
  constexpr variant_move_ctor_base(variant_move_ctor_base const&) = default;
  constexpr variant_move_ctor_base(variant_move_ctor_base&& other) noexcept(std::is_nothrow_move_constructible_v<base>) = default;

  constexpr variant_move_ctor_base& operator=(variant_move_ctor_base const&) = default;
  constexpr variant_move_ctor_base& operator=(variant_move_ctor_base&&) noexcept(std::is_nothrow_move_assignable_v<base>) = default;

  ~variant_move_ctor_base() = default;
};

template<typename... Ts>
struct variant_move_ctor_base<false, Ts...> : variant_dtor_base_t<Ts...> {
  using base = variant_dtor_base_t<Ts...>;
  using base::base;
  constexpr variant_move_ctor_base() noexcept(std::is_nothrow_default_constructible_v<base>) = default;
  constexpr variant_move_ctor_base(variant_move_ctor_base const&) = default;

  constexpr variant_move_ctor_base& operator=(variant_move_ctor_base const&) = default;
  constexpr variant_move_ctor_base& operator=(variant_move_ctor_base&&) noexcept(std::is_nothrow_move_assignable_v<base>) = default;

  constexpr variant_move_ctor_base(variant_move_ctor_base&& other) noexcept(variant_traits<Ts...>::noexcept_value::move_ctor) {
    this->index_ = other.index_;
    if (this->index_ != variant_npos) {
      this->move_stg(this->index_, std::move(other.storage));
    }
  };

  template<size_t... Is>
  void call_move_stg(std::index_sequence<Is...>, size_t index, storage_union<Ts...>&& other) {
    using dtype = void (*)(storage_union<Ts...>&, storage_union<Ts...>&&);
    static dtype movers[] = {[](storage_union<Ts...>& stg, storage_union<Ts...>&& other) {
      stg.template move_stg<Is>(std::move(other));
    }...};
    movers[index](this->storage, std::move(other));
  }

  constexpr void move_stg(size_t index, storage_union<Ts...>&& other) {
    call_move_stg(std::index_sequence_for<Ts...>{}, index, std::move(other));
  }

  ~variant_move_ctor_base() = default;
};

template<typename... Ts>
using variant_move_ctor_base_t = variant_move_ctor_base<variant_traits<Ts...>::trivial::move_ctor, Ts...>;
