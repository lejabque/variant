#pragma once

template<bool has_default_ctor = true>
struct enable_default_ctor {};

template<>
struct enable_default_ctor<false> {
  constexpr enable_default_ctor() noexcept = delete;
  constexpr enable_default_ctor(enable_default_ctor const&) noexcept = default;
  constexpr enable_default_ctor(enable_default_ctor&&) noexcept = default;

  constexpr enable_default_ctor& operator=(enable_default_ctor const&) noexcept = default;
  constexpr enable_default_ctor& operator=(enable_default_ctor&&) noexcept = default;
};

template<bool has_copy_assign = true>
struct enable_copy_assign {};

template<>
struct enable_copy_assign<false> {
  constexpr enable_copy_assign() noexcept = default;
  constexpr enable_copy_assign(enable_copy_assign const&) noexcept = default;
  constexpr enable_copy_assign(enable_copy_assign&&) noexcept = default;

  constexpr enable_copy_assign& operator=(enable_copy_assign const&) noexcept = delete;
  constexpr enable_copy_assign& operator=(enable_copy_assign&&) noexcept = default;
};

template<bool has_move_assign = true>
struct enable_move_assign {};

template<>
struct enable_move_assign<false> {
  constexpr enable_move_assign() noexcept = default;
  constexpr enable_move_assign(enable_move_assign const&) noexcept = default;
  constexpr enable_move_assign(enable_move_assign&&) noexcept = default;

  constexpr enable_move_assign& operator=(enable_move_assign const&) noexcept = default;
  constexpr enable_move_assign& operator=(enable_move_assign&&) noexcept = delete;
};

