#pragma once
#include "variant_dtor.h"

namespace variant_utils {
template<bool is_trivial, typename... Ts>
struct variant_move_ctor_base : variant_dtor_base_t<Ts...> {
  using base = variant_dtor_base_t<Ts...>;
  using base::base;
  using base::emplace;
  constexpr variant_move_ctor_base() = default;
  constexpr variant_move_ctor_base(variant_move_ctor_base const&) = default;
  constexpr variant_move_ctor_base(variant_move_ctor_base&& other) = default;

  constexpr variant_move_ctor_base& operator=(variant_move_ctor_base const&) = default;
  constexpr variant_move_ctor_base& operator=(variant_move_ctor_base&&) = default;

  ~variant_move_ctor_base() = default;
};

template<typename... Ts>
struct variant_move_ctor_base<false, Ts...> : variant_dtor_base_t<Ts...> {
  using base = variant_dtor_base_t<Ts...>;
  using base::base;
  using base::emplace;
  constexpr variant_move_ctor_base() = default;
  constexpr variant_move_ctor_base(variant_move_ctor_base const&) = default;

  constexpr variant_move_ctor_base& operator=(variant_move_ctor_base const&) = default;
  constexpr variant_move_ctor_base& operator=(variant_move_ctor_base&&) = default;

  constexpr variant_move_ctor_base(variant_move_ctor_base&& other) noexcept(variant_traits<Ts...>::noexcept_value::move_ctor) {
    this->index_ = other.index_;
    if (this->index_ != variant_npos) {
      visit_indexed([this, &other](auto&& other_value, auto other_index) {
        this->storage.template move_stg<other_index>(std::move(other.storage));
      }, std::move(other));
    }
  };

  ~variant_move_ctor_base() = default;
};

template<typename... Ts>
using variant_move_ctor_base_t = variant_move_ctor_base<variant_traits<Ts...>::trivial::move_ctor, Ts...>;
} // namespace variant_utils
