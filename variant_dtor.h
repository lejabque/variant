#pragma once

#include "storage_union.h"

inline constexpr size_t variant_npos = -1;

template<bool is_trivial, typename... Ts>
struct variant_dtor_base {
  constexpr variant_dtor_base() noexcept(std::is_nothrow_default_constructible_v<storage_union<Ts...>>) = default;
  constexpr explicit variant_dtor_base(bool) noexcept: storage(false) {};

  constexpr variant_dtor_base(variant_dtor_base const& other) = default;
  constexpr variant_dtor_base(variant_dtor_base&& other) noexcept = default;

  constexpr variant_dtor_base& operator=(variant_dtor_base const&) = default;
  constexpr variant_dtor_base& operator=(variant_dtor_base&&) noexcept(((
      std::is_nothrow_move_constructible_v<Ts> && std::is_nothrow_move_assignable_v<Ts>) && ...)) = default;

  template<typename U, typename... Args>
  constexpr explicit variant_dtor_base(in_place_type_t<U> in_place_flag, Args&& ... args)
      : storage(in_place_flag, std::forward<Args>(args)...),
        index_(get_index_of_type_v<U, Ts...>) {}

  template<size_t I, typename... Args>
  constexpr explicit variant_dtor_base(in_place_index_t<I> in_place_flag, Args&& ... args)
      : storage(in_place_flag, std::forward<Args>(args)...),
        index_(I) {}

  ~variant_dtor_base() = default;

  constexpr void destroy_stg(size_t index) {}

  storage_union<Ts...> storage;
  size_t index_ = 0;
};

template<typename... Ts>
struct variant_dtor_base<false, Ts...> {
  constexpr variant_dtor_base() noexcept(std::is_nothrow_default_constructible_v<storage_union<Ts...>>) = default;
  constexpr explicit variant_dtor_base(bool) noexcept: storage(false) {};

  constexpr variant_dtor_base(variant_dtor_base const& other) = default;
  constexpr variant_dtor_base(variant_dtor_base&& other) noexcept = default;

  constexpr variant_dtor_base& operator=(variant_dtor_base const&) = default;
  constexpr variant_dtor_base& operator=(variant_dtor_base&&) noexcept(((
      std::is_nothrow_move_constructible_v<Ts> && std::is_nothrow_move_assignable_v<Ts>) && ...)) = default;

  template<typename U, typename... Args>
  constexpr explicit variant_dtor_base(in_place_type_t<U> in_place_flag, Args&& ... args)
      : storage(in_place_flag, std::forward<Args>(args)...),
        index_(get_index_of_type_v<U, Ts...>) {}

  template<size_t I, typename... Args>
  constexpr explicit variant_dtor_base(in_place_index_t<I> in_place_flag, Args&& ... args)
      : storage(in_place_flag, std::forward<Args>(args)...),
        index_(I) {}

  ~variant_dtor_base() {
    // TODO: trivially: сделать базы
    if (index_ != variant_npos) {
      destroy_stg(index_);
    }
  }

  template<size_t... Is>
  void call_destroy_stg(std::index_sequence<Is...>, size_t index) {
    using dtype = void (*)(storage_union<Ts...>&);
    static constexpr dtype destroyers[] = {
        [](storage_union<Ts...>& stg) { stg.template destroy_stg<Is>(); }...
    };
    destroyers[index](storage);
  }

  constexpr void destroy_stg(size_t index) {
    call_destroy_stg(std::index_sequence_for<Ts...>{}, index);
  }
  storage_union<Ts...> storage;
  size_t index_ = 0;
};
