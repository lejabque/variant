#pragma once
#include "variant_dtor.h"

namespace variant_utils {
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
      // this->move_stg(this->index_, std::move(other.storage));
      auto visiter = [this, &other](auto&& other_value, auto other_index) {
        constexpr size_t other_index_v = decltype(other_index)::value;
        this->storage.template move_stg<other_index_v>(std::move(other.storage));
      };
      visit_indexed(visiter, other);
    }
  };

  ~variant_move_ctor_base() = default;
};

template<typename... Ts>
using variant_move_ctor_base_t = variant_move_ctor_base<variant_traits<Ts...>::trivial::move_ctor, Ts...>;
} // namespace variant_utils
