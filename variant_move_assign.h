#pragma once
#include "variant_move_ctor.h"

namespace variant_utils {
template<bool is_trivial, typename... Ts>
struct variant_move_assign_base : variant_move_ctor_base_t<Ts...> {
  using base = variant_move_ctor_base_t<Ts...>;
  using base::base;
  using base::emplace;
  constexpr variant_move_assign_base() = default;
  constexpr variant_move_assign_base(variant_move_assign_base const&) = default;
  constexpr variant_move_assign_base(variant_move_assign_base&&) = default;

  constexpr variant_move_assign_base& operator=(variant_move_assign_base const&) = default;
  constexpr variant_move_assign_base& operator=(variant_move_assign_base&&) = default;

  ~variant_move_assign_base() = default;
};

template<typename... Ts>
struct variant_move_assign_base<false, Ts...> : variant_move_ctor_base_t<Ts...> {
  using base = variant_move_ctor_base_t<Ts...>;
  using base::base;
  using base::emplace;
  constexpr variant_move_assign_base() = default;
  constexpr variant_move_assign_base(variant_move_assign_base const&) = default;
  constexpr variant_move_assign_base(variant_move_assign_base&& other) = default;

  constexpr variant_move_assign_base& operator=(variant_move_assign_base const&) = default;
  constexpr variant_move_assign_base& operator=(variant_move_assign_base&& other) noexcept(variant_traits<Ts...>::noexcept_value::move_assign) {
    if (this->index_ == variant_npos && other.index_ == variant_npos) {
      // If both *this and rhs are valueless by exception, does nothing
      return *this;
    }
    if (other.index_ == variant_npos) {
      // Otherwise, if rhs is valueless, but *this is not, destroys the value contained in *this and makes it valueless
      this->destroy_stg();
      this->index_ = variant_npos;
      return *this;
    }
    visit_indexed([this, &other](auto& this_value, auto&& other_value, auto this_index, auto other_index) {
      if constexpr (this_index == other_index) {
        /*
         * Otherwise, if rhs holds the same alternative as *this, assigns std::get<j>(std::move(rhs)) to the value
         * contained in *this, with j being index(). If an exception is thrown, *this does not become valueless:
         * the value depends on the exception safety guarantee of the alternative's move assignment.
         */
        this_value = std::move(other_value);
      } else {
        /*
         * Otherwise (if rhs and *this hold different alternatives),
         * equivalent to this->emplace<rhs.index()>(get<rhs.index()>(std::move(rhs))).
         * If an exception is thrown by T_i's move constructor, *this becomes valueless_by_exception.
         */
        this->template emplace<other_index>(std::move(other_value));
      }
    }, *this, std::move(other));
    return *this;
  };

  ~variant_move_assign_base() = default;
};

template<typename... Ts>
using variant_move_assign_base_t = variant_move_assign_base<variant_traits<Ts...>::trivial::move_assign, Ts...>;
} // namespace variant_utils
