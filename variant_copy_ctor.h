#pragma once
#include "variant_dtor.h"

template<bool is_trivial, typename... Ts>
struct variant_copy_ctor_base
    : variant_dtor_base<(std::is_trivially_destructible_v<Ts>&& ...), Ts...> {
  using dtor_base = variant_dtor_base<(std::is_trivially_destructible_v<Ts>&& ...), Ts...>;
  using dtor_base::dtor_base;
  constexpr variant_copy_ctor_base() noexcept(std::is_nothrow_default_constructible_v<dtor_base>) = default;

  constexpr variant_copy_ctor_base(variant_copy_ctor_base const& other) = default;
  constexpr variant_copy_ctor_base(variant_copy_ctor_base&& other) noexcept = default;

  constexpr variant_copy_ctor_base& operator=(variant_copy_ctor_base const&) = default;
  constexpr variant_copy_ctor_base& operator=(variant_copy_ctor_base&&) noexcept(((
      std::is_nothrow_move_constructible_v<Ts> && std::is_nothrow_move_assignable_v<Ts>) && ...)) = default;

  ~variant_copy_ctor_base() = default;
};

template<typename... Ts>
struct variant_copy_ctor_base<false, Ts...>
    : variant_dtor_base<(std::is_trivially_destructible_v<Ts>&& ...), Ts...> {
  using dtor_base = variant_dtor_base<(std::is_trivially_destructible_v<Ts>&& ...), Ts...>;
  using dtor_base::dtor_base;
  constexpr variant_copy_ctor_base() noexcept(std::is_nothrow_default_constructible_v<dtor_base>) = default;
  constexpr variant_copy_ctor_base(variant_copy_ctor_base&& other) noexcept = default;

  constexpr variant_copy_ctor_base& operator=(variant_copy_ctor_base const&) = default;
  constexpr variant_copy_ctor_base& operator=(variant_copy_ctor_base&&) noexcept(((
      std::is_nothrow_move_constructible_v<Ts> && std::is_nothrow_move_assignable_v<Ts>) && ...)) = default;

  // https://godbolt.org/z/rWvn4f - копи-конструктор для нетривиальных не constexpr
  variant_copy_ctor_base(variant_copy_ctor_base const& other) : dtor_base(true) {
    this->index_ = other.index_;
    if (this->index_ != variant_npos) {
      this->copy_stg(this->index_, other.storage);
    }
  };

  template<size_t... Is>
  void call_copy_stg(std::index_sequence<Is...>, size_t index, storage_union<Ts...> const& other) {
    using dtype = void (*)(storage_union<Ts...>&, storage_union<Ts...> const&);
    static dtype copyers[] = {
        [](storage_union<Ts...>& stg, storage_union<Ts...> const& other) { stg.template copy_stg<Is>(other); }...
    };
    copyers[index](this->storage, other);
  }

  constexpr void copy_stg(size_t index, storage_union<Ts...> const& other) {
    call_copy_stg(std::index_sequence_for<Ts...>{}, index, other);
  }

  ~variant_copy_ctor_base() = default;
};
