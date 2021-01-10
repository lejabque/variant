#pragma once
#include "variant_traits.h"
#include "variant_utils.h"

namespace variant_utils {
template<bool is_trivial_dtor, typename T>
struct value_holder {
  using Type = T;
  constexpr value_holder() = default;
  constexpr value_holder(value_holder const&) = default;
  constexpr value_holder(value_holder&&) = default;

  constexpr value_holder& operator=(value_holder const&) = default;
  constexpr value_holder& operator=(value_holder&&) = default;

  template<typename... Args>
  constexpr explicit value_holder(in_place_index_t<0>, Args&& ... args)
      : obj(std::forward<Args>(args)...) {}

  template<typename OtherHolder>
  static constexpr void construct_value_holder(value_holder* holder, OtherHolder&& other) {
    new(holder) value_holder(std::forward<OtherHolder>(other));
  }

  void swap(value_holder& other) {
    using std::swap;
    swap(obj, other.obj);
  }

  constexpr T const& get_obj() const& noexcept {
    return obj;
  }

  constexpr T const&& get_obj() const&& noexcept {
    return obj;
  }

  constexpr T& get_obj()& noexcept {
    return obj;
  }

  constexpr T&& get_obj()&& noexcept {
    return obj;
  }

  ~value_holder() = default;

  T obj;
};

/*
 * Union implicitly удаляет деструктор, если есть non-trivial destructible альтернатива
 * А для non-trivial destructible дефолтный конструктор всё равно не может быть constexpr, потому что это non-literal типы
 * Поэтому делаю aligned_storage+placement new
*/
template<typename T>
struct value_holder<false, T> {
  using Type = T;
  value_holder() noexcept(std::is_nothrow_default_constructible_v<T>) {
    new(reinterpret_cast<T*>(&obj)) T();
  }

  constexpr value_holder(value_holder const&) = default;
  constexpr value_holder(value_holder&&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
  constexpr value_holder& operator=(value_holder const&) = default;
  constexpr value_holder& operator=(value_holder&&) noexcept(std::is_nothrow_move_assignable_v<T>) = default;

  template<typename OtherHolder>
  static constexpr void construct_value_holder(value_holder* holder, OtherHolder&& other) {
    new(holder) value_holder(in_place_index<0>, std::forward<OtherHolder>(other).get_obj());
  }

  template<typename... Args>
  explicit value_holder(in_place_index_t<0>, Args&& ... args) {
    new(&obj) T(std::forward<Args>(args)...);
  }

  ~value_holder() = default;

  constexpr T const& get_obj() const& noexcept {
    return *reinterpret_cast<T const*>(&obj);
  }

  constexpr T const&& get_obj() const&& noexcept {
    return *reinterpret_cast<T const*>(&obj);
  }

  constexpr T& get_obj()& noexcept {
    return *reinterpret_cast<T*>(&obj);
  }

  constexpr T&& get_obj()&& noexcept {
    return *reinterpret_cast<T*>(&obj);
  }

  std::aligned_storage_t<sizeof(T), alignof(T)> obj;
};

template<typename... Ts>
union storage_union {};

template<typename T, typename... Ts>
union storage_union<T, Ts...> {
  using Type = T;
  using value_holder_t = value_holder<std::is_trivially_destructible_v<T>, T>;

  constexpr storage_union() noexcept
      : stg() {};

  template<size_t I, typename... Args>
  constexpr explicit storage_union(in_place_index_t<I>, Args&& ... args)
      : stg(in_place_index<I - 1>, std::forward<Args>(args)...) {}

  template<typename... Args>
  constexpr explicit storage_union(in_place_index_t<0> in_place_flag, Args&& ... args)
      : value(in_place_flag, std::forward<Args>(args)...) {}

  template<size_t I>
  void copy_stg(storage_union const& other) {
    if constexpr (I == 0) {
      value_holder_t::construct_value_holder(&value, other.value);
    } else {
      stg.template copy_stg<I - 1>(other.stg);
    }
  }

  template<size_t I>
  constexpr void move_stg(storage_union&& other) {
    if constexpr (I == 0) {
      value_holder_t::construct_value_holder(&value, std::move(other.value));
    } else {
      stg.template move_stg<I - 1>(std::move(other.stg));
    }
  }

  template<size_t I, class... Args>
  void emplace_stg(Args&& ... args) {
    if constexpr (I == 0) {
      new(&value) value_holder_t(in_place_index<0>,
                                 std::forward<Args>(args)...);
    } else {
      stg.template emplace_stg<I - 1>(std::forward<Args>(args)...);
    }
  }

  constexpr storage_union<Ts...> const& get_stg() const& noexcept {
    return stg;
  }

  constexpr storage_union<Ts...> const&& get_stg() const&& noexcept {
    return stg;
  }

  constexpr storage_union<Ts...>& get_stg()& noexcept {
    return stg;
  }

  constexpr storage_union<Ts...>&& get_stg()&& noexcept {
    return stg;
  }

  constexpr T const& get_obj() const& noexcept {
    return value.get_obj();
  }

  constexpr T const&& get_obj() const&& noexcept {
    return value.get_obj();
  }

  constexpr T& get_obj()& noexcept {
    return value.get_obj();
  }

  constexpr T&& get_obj()&& noexcept {
    return value.get_obj();
  }

  template<size_t ind>
  constexpr decltype(auto) get_stg() const {
    if constexpr (ind == 0) {
      return get_obj();
    } else {
      return stg.template get_stg<ind - 1>();
    }
  }

  template<size_t ind>
  constexpr decltype(auto) get_stg() {
    if constexpr (ind == 0) {
      return get_obj();
    } else {
      return stg.template get_stg<ind - 1>();
    }
  }

  template<typename Target>
  constexpr decltype(auto) get_stg() {
    return this->template get_stg<type_index_v<Target, T, Ts...>>();
  }

  template<typename Target>
  constexpr decltype(auto) get_stg() const {
    return this->template get_stg<type_index_v<Target, T, Ts...>>();
  }

  value_holder_t value;
  storage_union<Ts...> stg;
};
} // namespace variant_utils
