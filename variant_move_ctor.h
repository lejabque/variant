#pragma once
#include "variant_copy_assign.h"

template<bool is_trivial, typename... Types>
struct variant_move_ctor_base
    : variant_copy_assign_base<(std::is_trivially_copy_assignable_v<Types> && ...), Types...> {
  using copy_assign_base = variant_copy_assign_base<(std::is_trivially_copy_assignable_v<Types> && ...), Types...>;
  using copy_assign_base::copy_assign_base;
  constexpr variant_move_ctor_base() noexcept = default;
  constexpr variant_move_ctor_base(variant_move_ctor_base const&) noexcept = default;

  constexpr variant_move_ctor_base(variant_move_ctor_base&& other) noexcept((std::is_nothrow_move_constructible_v<Types> && ...)) = default;

  ~variant_move_ctor_base() = default;
};

template<typename... Types>
struct variant_move_ctor_base<false, Types...>
    : variant_copy_assign_base<(std::is_trivially_copy_assignable_v<Types> && ...), Types...> {
  using copy_assign_base = variant_copy_assign_base<(std::is_trivially_copy_assignable_v<Types> && ...), Types...>;
  using copy_assign_base::copy_assign_base;
  constexpr variant_move_ctor_base() noexcept = default;
  constexpr variant_move_ctor_base(variant_move_ctor_base const&) = default;

  constexpr variant_move_ctor_base(variant_move_ctor_base&& other) noexcept((std::is_nothrow_move_constructible_v<Types> && ...)) {
    this->index_ = other.index_;
    if (this->index_ != variant_npos) {
      this->move_stg(this->index_, std::move(other.storage));
    }
  };

  template<size_t... Is>
  void call_move_stg(std::index_sequence<Is...>, size_t index, storage_union<Types...>&& other) {
    using dtype = void (*)(storage_union<Types...>&, storage_union<Types...>&&);
    static dtype movers[] = {[](storage_union<Types...>& stg, storage_union<Types...>&& other) {
      stg.template move_stg<Is>(std::move(other));
    }...};
    movers[index](this->storage, std::move(other));
  }

  constexpr void move_stg(size_t index, storage_union<Types...>&& other) {
    call_move_stg(std::index_sequence_for<Types...>{}, index, std::move(other));
  }

  ~variant_move_ctor_base() = default;
};
