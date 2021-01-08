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
        index_(get_index_of_type_v<U, Ts...>) {}

  template<size_t I, typename... Args>
  constexpr explicit variant_dtor_base(in_place_index_t<I> in_place_flag, Args&& ... args)
      : storage(in_place_flag, std::forward<Args>(args)...),
        index_(I) {}

  constexpr size_t index() const noexcept {
    return index_;
  }
  ~variant_dtor_base() = default;

  constexpr void destroy_stg(size_t index) {}

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
        index_(get_index_of_type_v<U, Ts...>) {}

  template<size_t I, typename... Args>
  constexpr explicit variant_dtor_base(in_place_index_t<I> in_place_flag, Args&& ... args)
      : storage(in_place_flag, std::forward<Args>(args)...),
        index_(I) {}

  ~variant_dtor_base() {
    if (index_ != variant_npos) {
      destroy_stg(index_);
    }
  }

  constexpr size_t index() const noexcept {
    return index_;
  }

  template<size_t... Is>
  void call_destroy_stg(std::index_sequence<Is...>, size_t index) {
    using dtype = void (*)(storage_t&);
    static constexpr dtype destroyers[] = {
        [](storage_t& stg) { stg.template destroy_stg<Is>(); }...
    };
    destroyers[index](storage);
  }

  constexpr void destroy_stg(size_t index) {
    call_destroy_stg(std::index_sequence_for<Ts...>{}, index);
  }
  storage_t storage;
  size_t index_ = 0;
};

template<size_t I, class T>
struct variant_stg_alternative;

template<size_t I, bool flag, template<bool, typename> typename base, typename... Ts>
struct variant_stg_alternative<I, base<flag, Ts...>> {
  using type = get_nth_type_t<I, Ts...>;
};

template<size_t I, bool flag, template<bool, typename> typename base, typename... Ts>
struct variant_stg_alternative<I, const base<flag, Ts...>> {
  using type = const get_nth_type_t<I, Ts...>;
};

template<size_t I, class T>
using variant_stg_alternative_t = typename variant_stg_alternative<I, T>::type;


template<class T>
struct variant_stg_indexes;

template<bool flag, template<bool, typename> typename base, typename... Ts>
struct variant_stg_indexes<base<flag, Ts...>> {
  using type = std::index_sequence_for<Ts...>;
};

template<bool flag, template<bool, typename> typename base, typename... Ts>
struct variant_stg_indexes<const base<flag, Ts...>> {
  using type = std::index_sequence_for<Ts...>;
};

template<class T>
using variant_stg_indexes_t = typename variant_stg_indexes<T>::type;

template<size_t ind, bool flag, template<bool, typename> typename base, typename... Ts>
constexpr get_nth_type_t<ind, Ts...>& get(base<flag, Ts...>& v) {
  return get_stg<ind>(v.storage);
}

template<size_t ind, bool flag, template<bool, typename> typename base, typename... Ts>
constexpr get_nth_type_t<ind, Ts...> const& get(base<flag, Ts...> const& v) {
  return get_stg<ind>(v.storage);
}

template<typename Visitor, typename... Variants>
struct stg_table_impl_last {
  template<size_t... Is>
  static constexpr auto get_func(std::index_sequence<Is...>) {
    return [](Visitor&& vis, Variants... vars) {
      return vis(value_holder_index<Is>{}...);
    };
  }
};

template<bool is_last, typename R, typename Visitor, size_t Current, typename... Variants>
struct stg_table_impl {
  template<size_t... Prefix, size_t... VariantIndexes>
  static auto make_table(std::index_sequence<Prefix...>, std::index_sequence<VariantIndexes...>) {
    return std::experimental::make_array(
        stg_table_impl<Current + 2 == sizeof...(Variants),
                       R,
                       Visitor,
                       Current + 1,
                       Variants...>::make_table(std::index_sequence<Prefix..., VariantIndexes>{},
                                                variant_stg_indexes_t<std::decay_t<get_nth_type_t<
                                                    Current + 1,
                                                    Variants...>>>{})...);
  }
};

template<typename R, typename Visitor, size_t Current, typename... Variants>
struct stg_table_impl<true, R, Visitor, Current, Variants...> {
  template<size_t... Prefix, size_t... VariantIndexes>
  static constexpr auto make_table(std::index_sequence<Prefix...>, std::index_sequence<VariantIndexes...>) {
    return std::experimental::make_array(
        stg_table_impl_last<Visitor, Variants...>::get_func(std::index_sequence<Prefix..., VariantIndexes>{})...
    ); // TODO: закешировать в static
  }
};

template<typename Table>
constexpr auto get_from_table(Table&& table) {
  return table;
}

template<typename Table, typename Variant, typename...Variants>
constexpr auto get_from_table(Table&& table, Variant const& var, Variants const& ... vars) {
  return get_from_table(table[var.index()], vars...);
}

template<class T> // TODO: костыль
using variant_index_t = value_holder_index<0>;

template<typename Visitor, typename... Variants>
constexpr decltype(auto) visit_stg(Visitor&& vis, Variants&& ... vars) {
  return get_from_table(stg_table_impl<sizeof...(Variants) == 1,
                                       std::invoke_result_t<Visitor,
                                                            variant_index_t<std::decay_t<Variants>> ...>,
                                       Visitor, 0,
                                       Variants&& ...>::make_table(std::index_sequence<>{},
                                                                   variant_stg_indexes_t<std::decay_t<get_nth_type_t<0,
                                                                                                                     Variants...>>>{}),
                        vars...)(std::forward<Visitor>(vis),
                                 std::forward<Variants>(vars)...);
}

template<typename... Ts>
using variant_dtor_base_t = variant_dtor_base<variant_traits<Ts...>::trivial::dtor, Ts...>;
