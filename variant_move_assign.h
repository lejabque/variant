#pragma once
#include "variant_move_ctor.h"

template<bool is_trivial, typename... Ts>
struct variant_move_assign_base
    : variant_move_ctor_base<(std::is_trivially_move_constructible_v<Ts>&& ...), Ts...> {
  using move_ctor_base = variant_move_ctor_base<(std::is_trivially_move_constructible_v<Ts>&& ...),
                                                Ts...>;
  using move_ctor_base::move_ctor_base;
  constexpr variant_move_assign_base() noexcept(std::is_nothrow_default_constructible_v<move_ctor_base>) = default;
  constexpr variant_move_assign_base(variant_move_assign_base const&) = default;
  constexpr variant_move_assign_base(variant_move_assign_base&&) noexcept(std::is_nothrow_move_constructible_v<
      move_ctor_base>) = default;

  constexpr variant_move_assign_base& operator=(variant_move_assign_base const&) = default;
  constexpr variant_move_assign_base& operator=(variant_move_assign_base&&) noexcept(((
      std::is_nothrow_move_constructible_v<Ts> && std::is_nothrow_move_assignable_v<Ts>) && ...)) = default;

  ~variant_move_assign_base() = default;
};

template<typename... Ts>
struct variant_move_assign_base<false, Ts...>
    : variant_move_ctor_base<(std::is_trivially_move_constructible_v<Ts>&& ...), Ts...> {
  using move_ctor_base = variant_move_ctor_base<(std::is_trivially_move_constructible_v<Ts>&& ...),
                                                Ts...>;
  using move_ctor_base::move_ctor_base;
  constexpr variant_move_assign_base() noexcept(std::is_nothrow_default_constructible_v<move_ctor_base>) = default;
  constexpr variant_move_assign_base(variant_move_assign_base const&) = default;
  constexpr variant_move_assign_base(variant_move_assign_base&& other) noexcept(std::is_nothrow_move_constructible_v<
      move_ctor_base>) = default;

  constexpr variant_move_assign_base& operator=(variant_move_assign_base const&) = default;
  constexpr variant_move_assign_base& operator=(variant_move_assign_base&& other) noexcept(((
      std::is_nothrow_move_constructible_v<Ts> && std::is_nothrow_move_assignable_v<Ts>) && ...)) {
    // TODO
    return *this;
  };

  ~variant_move_assign_base() = default;
};
