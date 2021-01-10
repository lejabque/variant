#pragma once
#include "variant_copy_ctor.h"

namespace variant_utils {
template<bool is_trivial, typename... Ts>
struct variant_copy_assign_base : variant_copy_ctor_base_t<Ts...> {
  using base = variant_copy_ctor_base_t<Ts...>;
  using base::base;

  constexpr variant_copy_assign_base() noexcept(variant_utils::variant_traits<Ts...>::noexcept_value::default_ctor)
      : variant_copy_assign_base(in_place_index<0>) {}
  constexpr variant_copy_assign_base(variant_copy_assign_base const&) = default;
  constexpr variant_copy_assign_base(variant_copy_assign_base&& other) = default;

  constexpr variant_copy_assign_base& operator=(variant_copy_assign_base const&) = default;
  constexpr variant_copy_assign_base& operator=(variant_copy_assign_base&&) = default;

  ~variant_copy_assign_base() = default;
};

template<typename... Ts>
struct variant_copy_assign_base<false, Ts...> : variant_copy_ctor_base_t<Ts...> {
  using base = variant_copy_ctor_base_t<Ts...>;
  using base::base;

  constexpr variant_copy_assign_base() noexcept(variant_utils::variant_traits<Ts...>::noexcept_value::default_ctor)
      : variant_copy_assign_base(in_place_index<0>) {}
  constexpr variant_copy_assign_base(variant_copy_assign_base const&) = default;
  constexpr variant_copy_assign_base(variant_copy_assign_base&& other) = default;

  constexpr variant_copy_assign_base& operator=(variant_copy_assign_base const& other) {
    if (this->index_ == variant_npos && other.index_ == variant_npos) {
      // If both *this and rhs are valueless by exception, does nothing.
      return *this;
    }
    if (other.index_ == variant_npos) {
      // Otherwise, if rhs is valueless, but *this is not, destroys the value contained in *this and makes it valueless.
      this->destroy_stg();
      this->index_ = variant_npos;
      return *this;
    }
    visit_indexed([this, &other](auto& this_value, auto const& other_value, auto this_index, auto other_index) {
      if constexpr (this_index == other_index) {
        /*
         * Otherwise, if rhs holds the same alternative as *this,
         * assigns the value contained in rhs to the value contained in *this.
         * If an exception is thrown, *this does not become valueless:
         */
        this_value = other_value;
      } else {
        if constexpr(std::is_nothrow_copy_constructible_v<types_at_t<other_index, Ts...>>
            || !std::is_nothrow_move_constructible_v<types_at_t<other_index, Ts...>>) {
          /*
           * Otherwise, if the alternative held by rhs is either nothrow copy constructible or not nothrow move constructible
           * (as determined by std::is_nothrow_copy_constructible and std::is_nothrow_move_constructible, respectively),
           * equivalent to this->emplace<rhs.index()>(get<rhs.index()>(rhs)).
           */
          this->template emplace<other_index>(other_value);
        } else {
          // Otherwise, equivalent to this->operator=(variant(rhs)). Note that *this may become valueless_by_exception as described in (2).
          this->operator=(variant_copy_assign_base(other));
        }
      }
    }, *this, other);
    return *this;
  };
  constexpr variant_copy_assign_base& operator=(variant_copy_assign_base&&) = default;

  ~variant_copy_assign_base() = default;
};

template<typename... Ts>
using variant_copy_assign_base_t = variant_copy_assign_base<variant_traits<Ts...>::trivial::copy_assign, Ts...>;
} // namespace variant_utils
