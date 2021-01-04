#pragma once
#include "enable_ctor_base.h"
#include "variadic_utils.h"
#include "variant_move_ctor.h"

template<typename... Types>
struct variant_storage : variant_move_ctor_base<(std::is_trivially_move_constructible_v<Types> && ...), Types...>,
                         enable_default_ctor<std::is_default_constructible_v<get_nth_type_t<0, Types...>>>,
                         enable_copy_ctor<(std::is_copy_constructible_v<Types> && ...)>,
                         enable_move_ctor<(std::is_move_constructible_v<Types> && ...)> {
  constexpr variant_storage() noexcept = default;
  using move_ctor_base = variant_move_ctor_base<(std::is_trivially_move_constructible_v<Types> && ...), Types...>;
  using move_ctor_base::move_ctor_base;

  void swap(variant_storage& other) {

  }

  template<typename U, typename... Args>
  constexpr explicit variant_storage(in_place_type_t<U> in_place_flag, Args&& ... args)
      : move_ctor_base(in_place_flag, std::forward<Args>(args)...),
        enable_default_ctor<std::is_default_constructible_v<get_nth_type_t<0, Types...>>>{} {}

  template<size_t I, typename... Args>
  constexpr explicit variant_storage(in_place_index_t<I> in_place_flag, Args&& ... args)
      : move_ctor_base(in_place_flag, std::forward<Args>(args)...),
        enable_default_ctor<std::is_default_constructible_v<get_nth_type_t<0, Types...>>>{} {}

  constexpr variant_storage(variant_storage const&) = default;
  constexpr variant_storage(variant_storage&&) noexcept((std::is_nothrow_move_constructible_v<Types> && ...)) = default;

  template<size_t I, class... Args>
  get_nth_type_t<I, Types...>& emplace(Args&& ...args) {
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
};
