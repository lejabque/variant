#pragma once

template<bool is_trivial_dtor, typename T>
struct value_holder {
  constexpr value_holder() noexcept(std::is_nothrow_default_constructible_v<T>) = default;
  constexpr value_holder(value_holder const&) = default;
  constexpr value_holder(value_holder&&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
// TODO: noexcept of move
  template<typename... Args>
  constexpr explicit value_holder(in_place_index_t<0>, Args&& ... args)
      : obj(std::forward<Args>(args)...) {}

  constexpr operator T&() {
    return obj;
  }

  constexpr operator T() const {
    return obj;
  }

  constexpr operator T const&() const {
    return obj;
  }

  ~value_holder() = default;

  T obj;
};

// union не позволяет иметь дефолтный конструктор для non-trivial destructible типов
// TODO: доделать конструкторы
template<typename T>
struct value_holder<false, T> {
  value_holder() noexcept(std::is_nothrow_default_constructible_v<T>) {
    new(&obj) T();
  }

  constexpr value_holder(value_holder const&) = default;
  constexpr value_holder(value_holder&&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;

  template<typename... Args>
  explicit value_holder(in_place_index_t<0>, Args&& ... args) {
    new(&obj) T(std::forward<Args>(args)...);
  }

  ~value_holder() = default;

  operator T&() {
    return *reinterpret_cast<T*>(&obj);
  }

  operator T() const {
    return *reinterpret_cast<T*>(&obj);
  }

  operator T const&() const {
    return *reinterpret_cast<T const*>(&obj);
  }

  std::aligned_storage_t<sizeof(T), alignof(T)> obj;
};

template<typename T, typename... Ts>
union storage_union {

  constexpr explicit storage_union(bool) noexcept
      : dummy() {};

  constexpr storage_union() noexcept
      : obj() {};

  template<typename U, typename... Args>
  constexpr explicit storage_union(in_place_type_t<U> in_place_flag, Args&& ... args)
      : stg(in_place_flag, std::forward<Args>(args)...) {}

  template<typename... Args>
  constexpr explicit storage_union(in_place_type_t<T> in_place_flag, Args&& ... args)
      : obj(in_place_index_t<0>(), std::forward<Args>(args)...) {}

  template<size_t I, typename... Args>
  constexpr explicit storage_union(in_place_index_t<I> in_place_flag, Args&& ... args)
      : stg(in_place_index<I - 1>, std::forward<Args>(args)...) {}

  template<typename... Args>
  constexpr explicit storage_union(in_place_index_t<0> in_place_flag, Args&& ... args)
      : obj(in_place_flag, std::forward<Args>(args)...) {}

  template<size_t I>
  constexpr void destroy_stg() {
    if constexpr (I == 0) {
      reinterpret_cast<T*>(&obj.obj)->~T();
    } else {
      stg.template destroy_stg<I - 1>();
    }
  }

  template<size_t I>
  void copy_stg(storage_union const& other) {
    if constexpr (I == 0) {
      new(&obj) value_holder<std::is_trivially_destructible_v<T>, T>(other.obj);
    } else {
      stg.template copy_stg<I - 1>(other.stg);
    }
  }

  template<size_t I>
  constexpr void move_stg(storage_union&& other) {
    if constexpr (I == 0) {
      new(&obj) value_holder<std::is_trivially_destructible_v<T>, T>(std::move(other.obj));
    } else {
      stg.template move_stg<I - 1>(std::move(other.stg));
    }
  }

  template<size_t I, class... Args>
  void emplace_stg(Args&& ... args) {
    if constexpr (I == 0) {
      new(&obj) value_holder<std::is_trivially_destructible_v<T>, T>(in_place_index_t<0>(),
                                                                     std::forward<Args>(args)...);
    } else {
      stg.template emplace_stg<I - 1>(std::forward<Args>(args)...);
    }
  }

  value_holder<std::is_trivially_destructible_v<T>, T> obj;
  storage_union<Ts...> stg;
  bool dummy;
};

template<typename T>
union storage_union<T> {
  constexpr explicit storage_union(bool) noexcept
      : dummy{false} {};
  constexpr storage_union() noexcept
      : obj() {};

  template<typename... Args>
  constexpr explicit storage_union(in_place_type_t<T> in_place_flag, Args&& ... args)
      : obj(in_place_index_t<0>(), std::forward<Args>(args)...) {}

  template<typename... Args>
  constexpr explicit storage_union(in_place_index_t<0> in_place_flag, Args&& ... args)
      : obj(in_place_flag, std::forward<Args>(args)...) {}

  template<size_t I>
  constexpr void destroy_stg() {
    if constexpr (I == 0) {
      reinterpret_cast<T*>(&obj.obj)->~T();
    }
  }

  template<size_t I>
  void copy_stg(storage_union const& other) {
    if constexpr (I == 0) {
      new(&obj) value_holder<std::is_trivially_destructible_v<T>, T>(other.obj);
    }
  }

  template<size_t I>
  void move_stg(storage_union&& other) {
    if constexpr (I == 0) {
      new(&obj) value_holder<std::is_trivially_destructible_v<T>, T>(std::move(other.obj));
    }
  }

  template<size_t I, class... Args>
  void emplace_stg(Args&& ... args) {
    if constexpr (I == 0) {
      new(&obj) value_holder<std::is_trivially_destructible_v<T>, T>(in_place_index_t<0>(),
                                                                     std::forward<Args>(args)...);
    }
  }

  value_holder<std::is_trivially_destructible_v<T>, T> obj;
  bool dummy;
};

template<size_t ind, typename T, typename... Ts>
auto& get_stg_stg(storage_union<T, Ts...>& storage) {
  if constexpr (ind == 0) {
    return storage;
  } else {
    return get_stg_stg<ind - 1, Ts...>(storage.stg);
  }
}

template<size_t ind, typename T, typename... Ts>
constexpr get_nth_type_t<ind, T, Ts...> const& get_stg(storage_union<T, Ts...> const& storage) {
  if constexpr (ind == 0) {
    return storage.obj;
  } else {
    return get_stg<ind - 1, Ts...>(storage.stg);
  }
}

template<size_t ind, typename... Ts>
constexpr get_nth_type_t<ind, Ts...>& get_stg(storage_union<Ts...>& storage) {
  return const_cast<get_nth_type_t<ind, Ts...>&>(get_stg<ind, Ts...>(std::as_const(storage)));
}

template<typename Target, typename T, typename... Ts>
constexpr Target const& get_stg(storage_union<T, Ts...> const& storage) {
  if constexpr (std::is_same_v<Target, T>) {
    return storage.obj;
  } else {
    return get_stg<Target, Ts...>(storage.stg);
  }
}

template<typename Target, typename... Ts>
constexpr Target& get_stg(storage_union<Ts...>& storage) {
  return const_cast<Target&>(get_stg<Target, Ts...>(std::as_const(storage)));
}
