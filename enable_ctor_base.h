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

template<bool has_copy_ctor = true>
struct enable_copy_ctor {};

template<>
struct enable_copy_ctor<false> {
  constexpr enable_copy_ctor() noexcept = default;
  constexpr enable_copy_ctor(enable_copy_ctor const&) noexcept = delete;
  constexpr enable_copy_ctor(enable_copy_ctor&&) noexcept = default;

  constexpr enable_copy_ctor& operator=(enable_copy_ctor const&) noexcept = default;
  constexpr enable_copy_ctor& operator=(enable_copy_ctor&&) noexcept = default;
};

template<bool has_move_ctor = true>
struct enable_move_ctor {};

template<>
struct enable_move_ctor<false> {
  constexpr enable_move_ctor() noexcept = default;
  constexpr enable_move_ctor(enable_move_ctor const&) noexcept = default;
  constexpr enable_move_ctor(enable_move_ctor&&) noexcept = delete;

  constexpr enable_move_ctor& operator=(enable_move_ctor const&) noexcept = default;
  constexpr enable_move_ctor& operator=(enable_move_ctor&&) noexcept = default;
};


