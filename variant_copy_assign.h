#pragma once
#include "variant_copy_ctor.h"

template<bool is_trivial, typename... Types>
struct variant_copy_assign_base
    : variant_copy_ctor_base<(std::is_trivially_copy_constructible_v<Types>&& ...), Types...> {
  using copy_ctor_base = variant_copy_ctor_base<(std::is_trivially_copy_constructible_v<Types>&& ...),
                                                Types...>;
  using copy_ctor_base::copy_ctor_base;
  constexpr variant_copy_assign_base() noexcept = default;
  constexpr variant_copy_assign_base(variant_copy_assign_base const&) = default;
  constexpr variant_copy_assign_base& operator=(variant_copy_assign_base const&) = default;

  constexpr variant_copy_assign_base(variant_copy_assign_base&& other) noexcept = default;

  ~variant_copy_assign_base() = default;
};

template<typename... Types>
struct variant_copy_assign_base<false, Types...>
    : variant_copy_ctor_base<(std::is_trivially_copy_constructible_v<Types>&& ...), Types...> {
  using copy_ctor_base = variant_copy_ctor_base<(std::is_trivially_copy_constructible_v<Types>&& ...),
                                                Types...>;
  using copy_ctor_base::copy_ctor_base;
  constexpr variant_copy_assign_base() noexcept = default;
  constexpr variant_copy_assign_base(variant_copy_assign_base const&) = default;
  constexpr variant_copy_assign_base& operator=(variant_copy_assign_base const& rhs) noexcept {
    if (this->index_ == variant_npos && rhs.index_ == variant_npos) {
      // If both *this and rhs are valueless by exception, does nothing
      return *this;
    }
    if (rhs.index_ == variant_npos) {
      // Otherwise, if rhs is valueless, but *this is not, destroys the value contained in *this and makes it valueless.
      this->destroy_stg(this->index_);
      this->index_ = rhs.index_;
      return *this;
    }
    if (this->index_ == rhs.index_) {
      // Otherwise, if rhs holds the same alternative as *this,
      // assigns the value contained in rhs to the value contained in *this.
      // If an exception is thrown, *this does not become valueless:
      // the value depends on the exception safety guarantee of the alternative's copy assignment.
      this->storage.copy_assign_stg(rhs.storage);
      this->index_ = rhs.index_;
      return *this;
    }
    // TODO:
    // Otherwise, if the alternative held by rhs is either nothrow copy constructible or not nothrow move constructible
    // (as determined by std::is_nothrow_copy_constructible and std::is_nothrow_move_constructible, respectively),
    // equivalent to this->emplace<rhs.index()>(get<rhs.index()>(rhs)).

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

  constexpr variant_copy_assign_base(variant_copy_assign_base&& other) noexcept = default;

  ~variant_copy_assign_base() = default;
};
