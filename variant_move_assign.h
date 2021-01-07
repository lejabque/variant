#pragma once
#include "variant_move_ctor.h"

template<bool is_trivial, typename... Ts>
struct variant_move_assign_base
    : variant_move_ctor_base<(std::is_trivially_move_constructible_v<Ts>&& ...), Ts...> {
  using base = variant_move_ctor_base<(std::is_trivially_move_constructible_v<Ts>&& ...),
                                      Ts...>;
  using base::base;
  constexpr variant_move_assign_base() noexcept(std::is_nothrow_default_constructible_v<base>) = default;
  constexpr variant_move_assign_base(variant_move_assign_base const&) = default;
  constexpr variant_move_assign_base(variant_move_assign_base&&) noexcept(std::is_nothrow_move_constructible_v<base>) = default;

  constexpr variant_move_assign_base& operator=(variant_move_assign_base const&) = default;
  constexpr variant_move_assign_base& operator=(variant_move_assign_base&&) noexcept(((
      std::is_nothrow_move_constructible_v<Ts> && std::is_nothrow_move_assignable_v<Ts>) && ...)) = default;

  ~variant_move_assign_base() = default;
};

template<typename... Ts>
struct variant_move_assign_base<false, Ts...>
    : variant_move_ctor_base<(std::is_trivially_move_constructible_v<Ts>&& ...), Ts...> {
  using base = variant_move_ctor_base<(std::is_trivially_move_constructible_v<Ts>&& ...),
                                      Ts...>;
  using base::base;
  constexpr variant_move_assign_base() noexcept(std::is_nothrow_default_constructible_v<base>) = default;
  constexpr variant_move_assign_base(variant_move_assign_base const&) = default;
  constexpr variant_move_assign_base(variant_move_assign_base&& other) noexcept(std::is_nothrow_move_constructible_v<
      base>) = default;

  constexpr variant_move_assign_base& operator=(variant_move_assign_base const&) = default;
  constexpr variant_move_assign_base& operator=(variant_move_assign_base&& other) noexcept(((
      std::is_nothrow_move_constructible_v<Ts> && std::is_nothrow_move_assignable_v<Ts>) && ...)) {
    if (this->index_ == variant_npos && other.index_ == variant_npos) {
      return *this;
    }
    if (other.index_ == variant_npos) {
      this->destroy_stg(this->index_);
      this->index_ = other.index_;
      return *this;
    }
    auto visitor =
        [this, &other](auto this_index, auto other_index) {
          constexpr size_t this_index_v = decltype(this_index)::value;
          constexpr size_t other_index_v = decltype(other_index)::value;
          if constexpr (this_index_v == other_index_v) {
            get_stg<this_index_v>(this->storage) = std::move(get_stg<other_index_v>(other.storage));
          } else {
           if constexpr (this_index_v != variant_npos) {
              this->destroy_stg(this_index_v);
            }
            try {
              this->storage.template emplace_stg<other_index_v>(std::move(get_stg<other_index_v>(other.storage)));
            } catch (...) {
              this->index_ = variant_npos;
              throw;
            }
            this->index_ = other_index_v;
            // TODO: move
          }
        };
    visit_stg(visitor, *this, other);
    return *this;
  };

  ~variant_move_assign_base() = default;
};

// TODO: проблемы с implicit instantiation of undefined template
template<bool flag, typename... Ts>
struct variant_stg_indexes<variant_move_assign_base<flag, Ts...>> {
  using type = std::index_sequence_for<Ts...>;
};

template<bool flag, typename... Ts>
struct variant_stg_indexes<const variant_move_assign_base<flag, Ts...>> {
  using type = std::index_sequence_for<Ts...>;
};

template<class T>
using variant_stg_indexes_t = typename variant_stg_indexes<T>::type;