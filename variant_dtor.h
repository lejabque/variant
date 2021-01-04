#pragma once

#include "storage_union.h"

inline constexpr size_t variant_npos = -1;

template<bool is_trivial, typename... Types>
struct variant_dtor_base {
  constexpr variant_dtor_base() noexcept = default;
  constexpr explicit variant_dtor_base(bool) noexcept: storage(false) {};

  constexpr variant_dtor_base(variant_dtor_base const& other) = default;
  constexpr variant_dtor_base(variant_dtor_base&& other) noexcept = default;

  template<typename U, typename... Args>
  constexpr explicit variant_dtor_base(in_place_type_t<U> in_place_flag, Args&& ... args)
      : storage(in_place_flag, std::forward<Args>(args)...),
        index_(get_index_of_type_v<U, Types...>) {}

  template<size_t I, typename... Args>
  constexpr explicit variant_dtor_base(in_place_index_t<I> in_place_flag, Args&& ... args)
      : storage(in_place_flag, std::forward<Args>(args)...),
        index_(I) {}

  ~variant_dtor_base() = default;

  constexpr void destroy_stg(size_t index) {}

  storage_union<Types...> storage;
  size_t index_ = 0;
};

template<typename... Types>
struct variant_dtor_base<false, Types...> {
  constexpr variant_dtor_base() noexcept = default;
  constexpr explicit variant_dtor_base(bool) noexcept {};

  constexpr variant_dtor_base(variant_dtor_base const& other) = default;
  constexpr variant_dtor_base(variant_dtor_base&& other) noexcept = default;

  template<typename U, typename... Args>
  constexpr explicit variant_dtor_base(in_place_type_t<U> in_place_flag, Args&& ... args)
      : storage(in_place_flag, std::forward<Args>(args)...),
        index_(get_index_of_type_v<U, Types...>) {}

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
    using dtype = void (*)(storage_union<Types...>&);
    static constexpr dtype destroyers[] = {
        [](storage_union<Types...>& stg) { stg.template destroy_stg<Is>(); }...
    };
    destroyers[index](storage);
  }

  constexpr void destroy_stg(size_t index) {
    call_destroy_stg(std::index_sequence_for<Types...>{}, index);
  }
  storage_union<Types...> storage;
  size_t index_ = 0;
};
