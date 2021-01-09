#pragma once
#include "visit_table.h"
#include "storage_union.h"

inline constexpr size_t variant_npos = -1;

template<bool is_trivial, typename... Ts>
struct variant_dtor_base {
  using storage_t = storage_union<Ts...>;
  constexpr variant_dtor_base() noexcept(std::is_nothrow_default_constructible_v<storage_t>) = default;
  constexpr explicit variant_dtor_base(variant_dummy_t) noexcept(std::is_nothrow_constructible_v<storage_t,
                                                                                                 variant_dummy_t>)
      : storage(variant_dummy) {};

  constexpr variant_dtor_base(variant_dtor_base const& other) = default;
  constexpr variant_dtor_base(variant_dtor_base&& other) noexcept(std::is_nothrow_move_constructible_v<storage_t>) = default;

  constexpr variant_dtor_base& operator=(variant_dtor_base const&) = default;
  constexpr variant_dtor_base& operator=(variant_dtor_base&&) noexcept(std::is_nothrow_move_assignable_v<storage_t>) = default;

  template<typename U, typename... Args>
  constexpr explicit variant_dtor_base(in_place_type_t<U> in_place_flag, Args&& ... args)
      : storage(in_place_flag, std::forward<Args>(args)...),
        index_(type_index_v<U, Ts...>) {}

  template<size_t I, typename... Args>
  constexpr explicit variant_dtor_base(in_place_index_t<I> in_place_flag, Args&& ... args)
      : storage(in_place_flag, std::forward<Args>(args)...),
        index_(I) {}

  constexpr size_t index() const noexcept {
    return index_;
  }
  ~variant_dtor_base() = default;

  constexpr void destroy_stg() {}

  storage_t storage;
  size_t index_ = 0;
};

template<typename... Ts>
struct variant_dtor_base<false, Ts...> {
  using storage_t = storage_union<Ts...>;
  constexpr variant_dtor_base() noexcept(std::is_nothrow_default_constructible_v<storage_t>) = default;
  constexpr explicit variant_dtor_base(variant_dummy_t) noexcept(std::is_nothrow_constructible_v<storage_t,
                                                                                                 variant_dummy_t>)
      : storage(variant_dummy) {};

  constexpr variant_dtor_base(variant_dtor_base const& other) = default;
  constexpr variant_dtor_base(variant_dtor_base&& other) noexcept(std::is_nothrow_move_constructible_v<storage_t>) = default;

  constexpr variant_dtor_base& operator=(variant_dtor_base const&) = default;
  constexpr variant_dtor_base& operator=(variant_dtor_base&&) noexcept(std::is_nothrow_move_assignable_v<storage_t>) = default;

  template<typename U, typename... Args>
  constexpr explicit variant_dtor_base(in_place_type_t<U> in_place_flag, Args&& ... args)
      : storage(in_place_flag, std::forward<Args>(args)...),
        index_(type_index_v<U, Ts...>) {}

  template<size_t I, typename... Args>
  constexpr explicit variant_dtor_base(in_place_index_t<I> in_place_flag, Args&& ... args)
      : storage(in_place_flag, std::forward<Args>(args)...),
        index_(I) {}

  ~variant_dtor_base() {
    if (index_ != variant_npos) {
      destroy_stg();
    }
  }

  constexpr size_t index() const noexcept {
    return index_;
  }

  constexpr void destroy_stg() {
    auto visiter = [](auto& this_value, auto this_index) {
      using this_type = std::decay_t<decltype(this_value)>;
      this_value.~this_type();
    };
    visit_indexed(visiter, *this);
  }

  storage_t storage;
  size_t index_ = 0;
};

template<typename... Ts>
using variant_dtor_base_t = variant_dtor_base<variant_traits<Ts...>::trivial::dtor, Ts...>;
