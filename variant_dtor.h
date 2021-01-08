#pragma once
#include <experimental/array>
#include "storage_union.h"

inline constexpr size_t variant_npos = -1;

template<bool is_trivial, typename... Ts>
struct variant_dtor_base {
  using storage_t = storage_union<Ts...>;
  constexpr variant_dtor_base() noexcept(std::is_nothrow_default_constructible_v<storage_t>) = default;
  constexpr explicit variant_dtor_base(variant_dummy_t) noexcept(std::is_nothrow_constructible_v<storage_t,
                                                                                                 variant_dummy_t>)
      : storage(variant_dummy) {};

  constexpr variant_dtor_base(variant_dtor_base const& other) = default;
  constexpr variant_dtor_base(variant_dtor_base&& other) noexcept(std::is_nothrow_move_constructible_v<storage_t>) = default;

  constexpr variant_dtor_base& operator=(variant_dtor_base const&) = default;
  constexpr variant_dtor_base& operator=(variant_dtor_base&&) noexcept(std::is_nothrow_move_assignable_v<storage_t>) = default;

  template<typename U, typename... Args>
  constexpr explicit variant_dtor_base(in_place_type_t<U> in_place_flag, Args&& ... args)
      : storage(in_place_flag, std::forward<Args>(args)...),
        index_(type_index_v<U, Ts...>) {}

  template<size_t I, typename... Args>
  constexpr explicit variant_dtor_base(in_place_index_t<I> in_place_flag, Args&& ... args)
      : storage(in_place_flag, std::forward<Args>(args)...),
        index_(I) {}

  constexpr size_t index() const noexcept {
    return index_;
  }
  ~variant_dtor_base() = default;

  constexpr void destroy_stg() {}

  storage_t storage;
  size_t index_ = 0;
};

template<typename... Ts>
struct variant_dtor_base<false, Ts...> {
  using storage_t = storage_union<Ts...>;
  constexpr variant_dtor_base() noexcept(std::is_nothrow_default_constructible_v<storage_t>) = default;
  constexpr explicit variant_dtor_base(variant_dummy_t) noexcept(std::is_nothrow_constructible_v<storage_t,
                                                                                                 variant_dummy_t>)
      : storage(variant_dummy) {};

  constexpr variant_dtor_base(variant_dtor_base const& other) = default;
  constexpr variant_dtor_base(variant_dtor_base&& other) noexcept(std::is_nothrow_move_constructible_v<storage_t>) = default;

  constexpr variant_dtor_base& operator=(variant_dtor_base const&) = default;
  constexpr variant_dtor_base& operator=(variant_dtor_base&&) noexcept(std::is_nothrow_move_assignable_v<storage_t>) = default;

  template<typename U, typename... Args>
  constexpr explicit variant_dtor_base(in_place_type_t<U> in_place_flag, Args&& ... args)
      : storage(in_place_flag, std::forward<Args>(args)...),
        index_(type_index_v<U, Ts...>) {}

  template<size_t I, typename... Args>
  constexpr explicit variant_dtor_base(in_place_index_t<I> in_place_flag, Args&& ... args)
      : storage(in_place_flag, std::forward<Args>(args)...),
        index_(I) {}

  ~variant_dtor_base() {
    if (index_ != variant_npos) {
      destroy_stg();
    }
  }

  constexpr size_t index() const noexcept {
    return index_;
  }

  constexpr void destroy_stg() {
    auto visiter = [](auto& this_value, auto this_index) {
      using this_type = std::decay_t<decltype(this_value)>;
      this_value.~this_type();
    };
    visit_indexed(visiter, *this);
  }

  storage_t storage;
  size_t index_ = 0;
};

template<class T>
struct variant_stg_indexes;

template<bool flag, template<bool, typename...> typename base, typename... Ts>
struct variant_stg_indexes<base<flag, Ts...>> {
  using type = std::index_sequence_for<Ts...>;
};

template<class T>
using variant_stg_indexes_t = typename variant_stg_indexes<T>::type;

template<size_t ind, template<typename...> typename base, typename... Ts>
constexpr types_at_t<ind, Ts...>& get(base<Ts...>& v) {
  return get_stg<ind>(v.storage);
}

template<size_t ind, template<typename...> typename base, typename... Ts>
constexpr types_at_t<ind, Ts...> const& get(base<Ts...> const& v) {
  return get_stg<ind>(v.storage);
}

template<size_t ind, bool flag, template<bool, typename...> typename base, typename... Ts>
constexpr types_at_t<ind, Ts...>& get(base<flag, Ts...>& v) {
  return get_stg<ind>(v.storage);
}

template<size_t ind, bool flag, template<bool, typename...> typename base, typename... Ts>
constexpr types_at_t<ind, Ts...> const& get(base<flag, Ts...> const& v) {
  return get_stg<ind>(v.storage);
}

template<size_t I, typename T>
struct alternative;

template<size_t I, template<typename...> typename base, typename... Ts>
struct alternative<I, base<Ts...>> {
  using type = types_at_t<I, Ts...>;
};

template<size_t I, template<typename...> typename base, typename... Ts>
struct alternative<I, base<Ts...>&> {
  using type = types_at_t<I, Ts...>&;
};

template<size_t I, template<bool, typename...> typename base, bool flag, typename... Ts>
struct alternative<I, base<flag, Ts...>> {
  using type = types_at_t<I, Ts...>;
};

template<size_t I, template<bool, typename...> typename base, bool flag, typename... Ts>
struct alternative<I, base<flag, Ts...>&> {
  using type = types_at_t<I, Ts...>&;
};


template<size_t I, template<bool, typename...> typename base, bool flag, typename... Ts>
struct alternative<I, base<flag, Ts...> const&> {
  using type = types_at_t<I, Ts...> const&;
};


template<size_t I, typename T>
using alternative_t = typename alternative<I, T>::type;

template<class T>
struct alt_indexes;

template<template<typename...> typename base, typename... Ts>
struct alt_indexes<base<Ts...>> {
  using type = std::index_sequence_for<Ts...>;
};

template<template<bool, typename...> typename base, bool flag, typename... Ts>
struct alt_indexes<base<flag, Ts...>> {
  using type = std::index_sequence_for<Ts...>;
};

template<class T>
using alt_indexes_t = typename alt_indexes<T>::type;

template<size_t index, bool empty, typename... Ts>
struct alt_indexes_by_ind {
  using type = std::index_sequence<>;
};

template<size_t index, typename... Ts>
struct alt_indexes_by_ind<index, true, Ts...> {
  using type = alt_indexes_t<std::decay_t<types_at_t<index, Ts...>>>;
};

template<size_t index, typename... Ts>
using alt_indexes_by_ind_t = typename alt_indexes_by_ind<index, index < sizeof...(Ts), Ts...>::type;

template<typename T>
using value_holder_zero = value_holder_index<0>;

template<typename Table>
constexpr auto const& get_from_table(Table const& table) {
  return table;
}

template<typename Table, typename... Is>
constexpr auto const& get_from_table(Table const& table, size_t index, Is... indexes) {
  return get_from_table(table[index], indexes...);
}

template<bool indexed, typename R, typename Visitor, size_t CurrentLvl, typename PrefixSeq, typename VariantSeq, typename... Variants>
struct table_cache;

template<typename R, typename Visitor, size_t CurrentLvl, size_t... Prefix, typename... Variants>
struct table_cache<false, R,
                   Visitor,
                   CurrentLvl,
                   std::index_sequence<Prefix...>,
                   std::index_sequence<>,
                   Variants...> {
  static constexpr auto array = [](Visitor&& vis, Variants... vars) {
    return vis(get<Prefix>(std::forward<Variants>(vars))...);
  };
};

template<typename R, typename Visitor, size_t CurrentLvl, size_t... Prefix, typename... Variants>
struct table_cache<true, R,
                   Visitor,
                   CurrentLvl,
                   std::index_sequence<Prefix...>,
                   std::index_sequence<>,
                   Variants...> {
  static constexpr auto array = [](Visitor&& vis, Variants... vars) {
    return vis(get<Prefix>(std::forward<Variants>(vars))..., value_holder_index<Prefix>{}...);
  };
};

template<bool indexed, typename R, typename Visitor, size_t CurrentLvl, size_t... Prefix, size_t... VariantIndexes, typename... Variants>
struct table_cache<indexed, R,
                   Visitor,
                   CurrentLvl,
                   std::index_sequence<Prefix...>,
                   std::index_sequence<VariantIndexes...>,
                   Variants...> {
  static constexpr auto array = std::experimental::make_array(table_cache<indexed, R,
                                                                          Visitor,
                                                                          CurrentLvl + 1,
                                                                          std::index_sequence<Prefix...,
                                                                                              VariantIndexes>,
                                                                          alt_indexes_by_ind_t<CurrentLvl + 1,
                                                                                               Variants...>,
                                                                          Variants...>::array...
  );
};

template<bool indexed, typename R, typename Visitor, typename... Variants>
using table_cache_t = table_cache<indexed, R,
                                  Visitor, 0, std::index_sequence<>,
                                  alt_indexes_t<std::decay_t<types_at_t<0, Variants...>>>, Variants&& ...>;

template<typename Visitor, typename... Variants>
constexpr decltype(auto) visit_indexed(Visitor&& vis, Variants&& ... vars) {
  return get_from_table(table_cache_t<true,
                                      std::invoke_result_t<Visitor,
                                                           alternative_t<0, Variants>...,
                                                           value_holder_zero<Variants>...>,
                                      Visitor,
                                      Variants...>::array, vars.index()...)(std::forward<Visitor>(vis),
                                                                            std::forward<Variants>(vars)...);
}

template<typename... Ts>
using variant_dtor_base_t = variant_dtor_base<variant_traits<Ts...>::trivial::dtor, Ts...>;
