#pragma once

template<bool has_default_ctor = true>
struct enable_default_ctor {};

template<>
struct enable_default_ctor<false> {
  enable_default_ctor() = delete;
  enable_default_ctor(enable_default_ctor const&) = default;
  enable_default_ctor(enable_default_ctor&&) = default;
};

template<bool has_copy_ctor = true>
struct enable_copy_ctor {};

template<>
struct enable_copy_ctor<false> {
  enable_copy_ctor() = default;
  enable_copy_ctor(enable_copy_ctor const&) = delete;
  enable_copy_ctor(enable_copy_ctor&&) = default;
};


template<bool has_move_ctor = true>
struct enable_move_ctor {};

template<>
struct enable_move_ctor<false> {
  enable_move_ctor() = default;
  enable_move_ctor(enable_move_ctor const&) = default;
  enable_move_ctor(enable_move_ctor&&) = delete;
};


