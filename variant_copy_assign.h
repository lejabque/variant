#pragma once
#include "variant_copy_ctor.h"

template<bool is_trivial, typename... Ts>
struct variant_copy_assign_base
    : variant_copy_ctor_base<(std::is_trivially_copy_constructible_v<Ts>&& ...), Ts...> {
  using base = variant_copy_ctor_base<(std::is_trivially_copy_constructible_v<Ts>&& ...),
                                      Ts...>;
  using base::base;
  constexpr variant_copy_assign_base() noexcept(std::is_nothrow_default_constructible_v<base>) = default;
  constexpr variant_copy_assign_base(variant_copy_assign_base const&) = default;
  constexpr variant_copy_assign_base(variant_copy_assign_base&& other) noexcept((std::is_nothrow_move_constructible_v<Ts> && ...)) = default;

  constexpr variant_copy_assign_base& operator=(variant_copy_assign_base const&) = default;
  constexpr variant_copy_assign_base& operator=(variant_copy_assign_base&&) noexcept(((
      std::is_nothrow_move_constructible_v<Ts> && std::is_nothrow_move_assignable_v<Ts>) && ...)) = default;

  ~variant_copy_assign_base() = default;
};

template<typename... Ts>
struct variant_copy_assign_base<false, Ts...>
    : variant_copy_ctor_base<(std::is_trivially_copy_constructible_v<Ts>&& ...), Ts...> {
  using base = variant_copy_ctor_base<(std::is_trivially_copy_constructible_v<Ts>&& ...),
                                      Ts...>;
  using base::base;
  constexpr variant_copy_assign_base() noexcept(std::is_nothrow_default_constructible_v<base>) = default;
  constexpr variant_copy_assign_base(variant_copy_assign_base const&) = default;
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
    if (this->index_ == other.index_) {
      // Otherwise, if rhs holds the same alternative as *this,
      // assigns the value contained in rhs to the value contained in *this.
      // If an exception is thrown, *this does not become valueless:
      // the value depends on the exception safety guarantee of the alternative's copy assignment.
      this->storage.copy_assign_stg(other.storage);
      this->index_ = other.index_;
      return *this;
    }
//    auto visitor =
//        [this, &other](auto& this_value_holder,
//                     auto const& other_value_holder, auto this_index, auto other_index) {
//
//          using OtherT = typename decltype(other_value_holder)::Type;
//          if constexpr (decltype(this_index)::value == decltype(other_index)::value) {
//
//          } else {
//            if constexpr (std::is_nothrow_copy_constructible_v<OtherT>
//                || std::is_nothrow_move_constructible_v<OtherT>) {
//              // emplace
//            } else {
//              this->operator=(variant_copy_assign_base(other));
//            }
//          }
//        };
//    visit_stg(visitor, *this, other);
    return *this;
    // TODO:
    // Otherwise, if the alternative held by rhs is either nothrow copy constructible or not nothrow move constructible
    // (as determined by std::is_nothrow_copy_constructible and std::is_nothrow_move_constructible, respectively),
    // equivalent to this->emplace<rhs.index()>(get<rhs.index()>(rhs)).

//    if (std::is_nothrow_copy_constructible_v<>)
    //    if (this->index_ != variant_npos) {
//      if (other.index_ == variant_npos) {
//        this->storage.destroy_stg(this->index_);
//        this->index_ = other.index_;
//      } else {
//        if (this->index_ == other.index_) {
//          this->storage.assign_value(other.storage); // TODO: assign_value
//        } else {
//          if constexpr () {
//            // TODO: ??? this->emplace<other.index()>(get<other.index()>(other))
//          } else {
//            this->operator=(variant(other)); // TODO: ???
//          }
//        }
//      }
//    }
  };
  constexpr variant_copy_assign_base& operator=(variant_copy_assign_base&&) noexcept(((
      std::is_nothrow_move_constructible_v<Ts> && std::is_nothrow_move_assignable_v<Ts>) && ...)) = default;

  constexpr variant_copy_assign_base(variant_copy_assign_base&& other) noexcept((std::is_nothrow_move_constructible_v<Ts> && ...)) = default;

  ~variant_copy_assign_base() = default;
};


