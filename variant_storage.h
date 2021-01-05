#pragma once
#include "enable_ctor_base.h"
#include "variadic_utils.h"
#include "variant_move_assign.h"

template<typename... Ts>
struct variant_storage : variant_move_assign_base<((std::is_trivially_move_constructible_v<Ts>
    && std::is_trivially_move_assignable_v<Ts> && std::is_trivially_destructible_v<Ts>) && ...), Ts...>,
                         enable_default_ctor<std::is_default_constructible_v<get_nth_type_t<0, Ts...>>>,
                         enable_copy_ctor<(std::is_copy_constructible_v<Ts> && ...)>,
                         enable_move_ctor<(std::is_move_constructible_v<Ts> && ...)>,
                         enable_copy_assign<((std::is_copy_constructible_v<Ts>
                             && std::is_copy_assignable_v<Ts>) && ...)>,
                         enable_move_assign<((std::is_move_constructible_v<Ts>
                             && std::is_move_assignable_v<Ts>) && ...)> {
  using move_assign_base = variant_move_assign_base<((std::is_trivially_move_constructible_v<Ts>
      && std::is_trivially_move_assignable_v<Ts> && std::is_trivially_destructible_v<Ts>) && ...), Ts...>;
  constexpr variant_storage() noexcept(std::is_nothrow_default_constructible_v<move_assign_base>) = default;
  // using move_ctor_base::move_ctor_base;
  void swap(variant_storage& other) {
    if (this->index_ == variant_npos && other.index_ == variant_npos) {
      return;
    }
    if (this->index_ == other.index_) {
      this->swap_stg(this->index_, other.storage);
      return;
    }
    // TODO: else
    variant_storage tmp(std::move(other));
    other = std::move(*this);
    *this = std::move(tmp);
  }

  template<typename U, typename... Args>
  constexpr explicit variant_storage(in_place_type_t<U> in_place_flag, Args&& ... args)
      : move_assign_base(in_place_flag, std::forward<Args>(args)...),
        enable_default_ctor<std::is_default_constructible_v<get_nth_type_t<0, Ts...>>>{} {}

  template<size_t I, typename... Args>
  constexpr explicit variant_storage(in_place_index_t<I> in_place_flag, Args&& ... args)
      : move_assign_base(in_place_flag, std::forward<Args>(args)...),
        enable_default_ctor<std::is_default_constructible_v<get_nth_type_t<0, Ts...>>>{} {}

  constexpr variant_storage(variant_storage const&) = default;
  constexpr variant_storage(variant_storage&&) noexcept((std::is_nothrow_move_constructible_v<Ts> && ...)) = default;
  constexpr variant_storage& operator=(variant_storage const&) = default;
  constexpr variant_storage& operator=(variant_storage&&) noexcept(((std::is_nothrow_move_constructible_v<Ts>
      && std::is_nothrow_move_assignable_v<Ts>) && ...)) = default;

  template<size_t I, class... Args>
  get_nth_type_t<I, Ts...>& emplace(Args&& ...args) {
    if (this->index_ != variant_npos) {
      this->destroy_stg(this->index_);
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
 private:
  template<size_t... Is>
  void call_swap_stg(std::index_sequence<Is...>, size_t index, storage_union<Ts...>& other) {
    using dtype = void (*)(storage_union<Ts...>&, storage_union<Ts...>&);
    static dtype swapper[] = {[](storage_union<Ts...>& stg, storage_union<Ts...>& other) {
      stg.template swap_stg<Is>(other);
    }...};
    swapper[index](this->storage, other);
  }

  constexpr void swap_stg(size_t index, storage_union<Ts...>& other) {
    call_swap_stg(std::index_sequence_for<Ts...>{}, index, other);
  }
};
