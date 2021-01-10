#pragma once
#include "variant_move_assign.h"

namespace variant_utils {
template<bool is_trivial, typename... Ts>
struct variant_copy_ctor_base : variant_move_assign_base_t<Ts...> {
  using base = variant_move_assign_base_t<Ts...>;
  using base::base;

  constexpr variant_copy_ctor_base() = default;

  constexpr variant_copy_ctor_base(variant_copy_ctor_base const& other) = default;
  constexpr variant_copy_ctor_base(variant_copy_ctor_base&& other) = default;

  constexpr variant_copy_ctor_base& operator=(variant_copy_ctor_base const&) = default;
  constexpr variant_copy_ctor_base& operator=(variant_copy_ctor_base&&) = default;

  ~variant_copy_ctor_base() = default;
};

template<typename... Ts>
struct variant_copy_ctor_base<false, Ts...> : variant_move_assign_base_t<Ts...> {
  using base = variant_move_assign_base_t<Ts...>;
  using base::base;

  constexpr variant_copy_ctor_base() = default;

  // https://godbolt.org/z/rWvn4f - копи-конструктор для нетривиальных не constexpr
  variant_copy_ctor_base(variant_copy_ctor_base const& other) {
    this->index_ = other.index_;
    if (this->index_ != variant_npos) {
      visit_indexed([this, &other](auto const& other_value, auto other_index) {
        this->storage.template copy_stg<other_index>(other.storage);
      }, other);
    }
  };

  constexpr variant_copy_ctor_base(variant_copy_ctor_base&& other) = default;

  constexpr variant_copy_ctor_base& operator=(variant_copy_ctor_base const&) = default;
  constexpr variant_copy_ctor_base& operator=(variant_copy_ctor_base&&) = default;

  ~variant_copy_ctor_base() = default;
};

template<typename... Ts>
using variant_copy_ctor_base_t = variant_copy_ctor_base<variant_traits<Ts...>::trivial::copy_ctor, Ts...>;
} // namespace variant_utils
