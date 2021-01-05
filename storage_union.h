#pragma once

template<bool is_trivial_dtor, typename T>
struct value_holder {
  constexpr value_holder() noexcept(std::is_nothrow_default_constructible_v<T>) = default;
  constexpr value_holder(value_holder const&) = default;
  constexpr value_holder(value_holder&&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;

  constexpr value_holder& operator=(value_holder const&) = default;
  constexpr value_holder& operator=(value_holder&&) noexcept((
      std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>)) = default;
// TODO: noexcept of move
  template<typename... Args>
  constexpr explicit value_holder(in_place_index_t<0>, Args&& ... args)
      : obj(std::forward<Args>(args)...) {}

  static constexpr void construct_value_holder(value_holder* holder, value_holder const& other) {
    new(holder) value_holder(other);
  }

  static constexpr void construct_value_holder(value_holder* holder, value_holder&& other) {
    new(holder) value_holder(std::move(other));
  }

  template<typename R, typename Visitor>
  constexpr R apply_func(Visitor&& vis) {
    return vis(obj);
  }

  void swap(value_holder& other) {
    using std::swap;
    swap(obj, other.obj);
  }

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
    new(reinterpret_cast<T*>(&obj)) T();
  }

  constexpr value_holder(value_holder const&) = default;
  constexpr value_holder(value_holder&&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
  constexpr value_holder& operator=(value_holder const&) = default;
  constexpr value_holder& operator=(value_holder&&) noexcept((
      std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>)) = default;

  static constexpr void construct_value_holder(value_holder* holder, value_holder const& other) {
    new(&holder->obj) T(*reinterpret_cast<T const*>(&other.obj));
  }

  static constexpr void construct_value_holder(value_holder* holder, value_holder&& other) {
    new(&holder->obj) T(std::move(*reinterpret_cast<T const*>(&other.obj)));
  }

  template<typename... Args>
  explicit value_holder(in_place_index_t<0>, Args&& ... args) {
    new(&obj) T(std::forward<Args>(args)...);
  }

  void swap(value_holder& other) {
    using std::swap;
    swap(*reinterpret_cast<T*>(&obj), *reinterpret_cast<T*>(&other.obj));
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

  template<typename R, typename Visitor>
  constexpr R apply_func(Visitor&& vis) {
    return vis(*reinterpret_cast<T*>(&obj));
  }

  std::aligned_storage_t<sizeof(T), alignof(T)> obj;
};

template<typename... Ts>
union storage_union;

template<>
union storage_union<> {
  bool dummy;
};

template<typename T, typename... Ts>
union storage_union<T, Ts...> {
  constexpr explicit storage_union(bool) noexcept
      : dummy() {};

  constexpr storage_union()
  noexcept(std::is_nothrow_default_constructible_v<value_holder<std::is_trivially_destructible_v<T>, T>>)
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
      value_holder<std::is_trivially_destructible_v<T>, T>::construct_value_holder(&obj, other.obj);
    } else {
      stg.template copy_stg<I - 1>(other.stg);
    }
  }

  template<size_t I>
  constexpr void swap_stg(storage_union& other) {
    if constexpr (I == 0) {
      obj.swap(other.obj);
    } else {
      stg.template swap_stg<I - 1>(other.stg);
    }
  }

  template<size_t I>
  constexpr void move_stg(storage_union&& other) {
    if constexpr (I == 0) {
      value_holder<std::is_trivially_destructible_v<T>, T>::construct_value_holder(&obj, std::move(other.obj));
    } else {
      stg.template move_stg<I - 1>(std::move(other.stg));
    }
  }

  template<typename R, size_t I, typename Visitor>
  constexpr R apply_func(Visitor&& vis) {
    if constexpr (I == 0) {
      return obj.template apply_func<R>(std::forward<Visitor>(vis));
    } else {
      return stg.template apply_func<R, I - 1>(std::forward<Visitor>(vis));
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