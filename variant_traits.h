#pragma once
#include "variadic_utils.h"
#include "enable_ctor_base.h"

template<typename... Ts>
struct variant_traits {
  struct noexcept_value {
    static constexpr bool default_ctor = (std::is_nothrow_default_constructible_v<types_at_t<0, Ts...>>);
    static constexpr bool move_assign = ((std::is_nothrow_move_constructible_v<Ts>
        && std::is_nothrow_move_assignable_v<Ts>) && ...);
    static constexpr bool move_ctor = (std::is_nothrow_move_constructible_v<Ts> && ...);
    static constexpr bool swap = ((std::is_nothrow_move_constructible_v<Ts> &&
        std::is_nothrow_swappable_v<Ts>) && ...);
  };
  struct trivial {
    static constexpr bool copy_assign = ((std::is_trivially_copy_constructible_v<Ts>
        && std::is_trivially_copy_assignable_v<Ts> && std::is_trivially_destructible_v<Ts>)&& ...);
    static constexpr bool copy_ctor = (std::is_trivially_copy_constructible_v<Ts>&& ...);
    static constexpr bool move_assign = ((std::is_trivially_move_constructible_v<Ts>
        && std::is_trivially_move_assignable_v<Ts> && std::is_trivially_destructible_v<Ts>) && ...);
    static constexpr bool move_ctor = (std::is_trivially_move_constructible_v<Ts>&& ...);
    static constexpr bool dtor = (std::is_trivially_destructible_v<Ts>&& ...);
  };
};

template<typename... Ts>
struct enable_bases
    : enable_default_ctor<std::is_default_constructible_v<types_at_t<0, Ts...>>>,
      enable_copy_ctor<(std::is_copy_constructible_v<Ts> && ...)>,
      enable_move_ctor<(std::is_move_constructible_v<Ts> && ...)>,
      enable_copy_assign<((std::is_copy_constructible_v<Ts>
          && std::is_copy_assignable_v<Ts>) && ...)>,
      enable_move_assign<((std::is_move_constructible_v<Ts>
          && std::is_move_assignable_v<Ts>) && ...)> {
};