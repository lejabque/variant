#pragma once
#include "variant_copy_ctor.h"

template<bool is_trivial, typename... Ts>
struct variant_copy_assign_base : variant_copy_ctor_base_t<Ts...> {
  using base = variant_copy_ctor_base_t<Ts...>;
  using base::base;
  constexpr variant_copy_assign_base() noexcept(std::is_nothrow_default_constructible_v<base>) = default;
  constexpr variant_copy_assign_base(variant_copy_assign_base const&) = default;
  constexpr variant_copy_assign_base(variant_copy_assign_base&& other) noexcept(std::is_nothrow_move_constructible_v<
      base>) = default;

  constexpr variant_copy_assign_base& operator=(variant_copy_assign_base const&) = default;
  constexpr variant_copy_assign_base& operator=(variant_copy_assign_base&&) noexcept(std::is_nothrow_move_assignable_v<
      base>) = default;

  ~variant_copy_assign_base() = default;
};

template<typename... Ts>
struct variant_copy_assign_base<false, Ts...> : variant_copy_ctor_base_t<Ts...> {
  using base = variant_copy_ctor_base_t<Ts...>;
  using base::base;
  constexpr variant_copy_assign_base() noexcept(std::is_nothrow_default_constructible_v<base>) = default;
  constexpr variant_copy_assign_base(variant_copy_assign_base const&) = default;
  constexpr variant_copy_assign_base(variant_copy_assign_base&& other) noexcept(std::is_nothrow_move_constructible_v<
      base>) = default;

  constexpr variant_copy_assign_base& operator=(variant_copy_assign_base const& other) {
    if (this->index_ == variant_npos && other.index_ == variant_npos) {
      // If both *this and rhs are valueless by exception, does nothing
      return *this;
    }
    if (other.index_ == variant_npos) {
      // Otherwise, if rhs is valueless, but *this is not, destroys the value contained in *this and makes it valueless.
      this->destroy_stg(this->index_);
      this->index_ = other.index_;
      return *this;
    }
    auto visitor =
        [this, &other](auto this_index, auto other_index) {
          constexpr size_t this_index_v = decltype(this_index)::value;
          constexpr size_t other_index_v = decltype(other_index)::value;
          if constexpr (this_index_v == other_index_v) {
            get_stg<this_index_v>(this->storage) = get_stg<other_index_v>(other.storage);
          } else {
            if constexpr(std::is_nothrow_copy_constructible_v<get_nth_type_t<other_index_v, Ts...>>
                || !std::is_nothrow_move_constructible_v<get_nth_type_t<other_index_v, Ts...>>) {
              if constexpr (this_index_v != variant_npos) {
                this->destroy_stg(this_index_v);
              }
              try {
                this->storage.template emplace_stg<other_index_v>(get_stg<other_index_v>(other.storage));
              } catch (...) {
                this->index_ = variant_npos;
                throw;
              }
              this->index_ = other_index_v;
            } else {
              this->operator=(variant_copy_assign_base(other));
            }
          }
        };
    visit_stg(visitor, *this, other);
    return *this;
  };
  constexpr variant_copy_assign_base& operator=(variant_copy_assign_base&&) noexcept(std::is_nothrow_move_assignable_v<base>) = default;

  ~variant_copy_assign_base() = default;
};

template<typename... Ts>
using variant_copy_assign_base_t = variant_copy_assign_base<variant_traits<Ts...>::trivial::copy_assign, Ts...>;

// TODO: проблемы с implicit instantiation of undefined template
template<bool flag, typename... Ts>
struct variant_stg_indexes<variant_copy_assign_base<flag, Ts...>> {
  using type = std::index_sequence_for<Ts...>;
};

template<bool flag, typename... Ts>
struct variant_stg_indexes<const variant_copy_assign_base<flag, Ts...>> {
  using type = std::index_sequence_for<Ts...>;
};

template<class T>
using variant_stg_indexes_t = typename variant_stg_indexes<T>::type;
