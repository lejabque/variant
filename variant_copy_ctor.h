#pragma once
#include "variant_dtor.h"

template<bool is_trivial, typename... Types>
struct variant_copy_ctor_base
    : variant_dtor_base<(std::is_trivially_destructible_v<Types>&& ...), Types...> {
  using dtor_base = variant_dtor_base<(std::is_trivially_destructible_v<Types>&& ...), Types...>;
  using dtor_base::dtor_base;
  constexpr variant_copy_ctor_base() noexcept = default;

  constexpr variant_copy_ctor_base(variant_copy_ctor_base const& other) = default;
  constexpr variant_copy_ctor_base(variant_copy_ctor_base&& other) noexcept = default;

  ~variant_copy_ctor_base() = default;
};

template<typename... Types>
struct variant_copy_ctor_base<false, Types...>
    : variant_dtor_base<(std::is_trivially_destructible_v<Types>&& ...), Types...> {
  using dtor_base = variant_dtor_base<(std::is_trivially_destructible_v<Types>&& ...), Types...>;
  using dtor_base::dtor_base;
  constexpr variant_copy_ctor_base() noexcept = default;
  constexpr variant_copy_ctor_base(variant_copy_ctor_base&& other) noexcept = default;

  // https://godbolt.org/z/rWvn4f - копи-конструктор для нетривиальных не constexpr
  variant_copy_ctor_base(variant_copy_ctor_base const& other) : dtor_base(true) {
    this->index_ = other.index_;
    if (this->index_ != variant_npos) {
      this->copy_stg(this->index_, other.storage);
    }
  };

  template<size_t... Is>
  void call_copy_stg(std::index_sequence<Is...>, size_t index, storage_union<Types...> const& other) {
    using dtype = void (*)(storage_union<Types...>&, storage_union<Types...> const&);
    static dtype copyers[] = {
        [](storage_union<Types...>& stg, storage_union<Types...> const& other) { stg.template copy_stg<Is>(other); }...
    };
    copyers[index](this->storage, other);
  }

  constexpr void copy_stg(size_t index, storage_union<Types...> const& other) {
    call_copy_stg(std::index_sequence_for<Types...>{}, index, other);
  }

  ~variant_copy_ctor_base() = default;
};
