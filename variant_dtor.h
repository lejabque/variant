#pragma once

#include "storage_union.h"

inline constexpr size_t variant_npos = -1;

template<bool is_trivial, typename... Ts>
struct variant_dtor_base {
  constexpr variant_dtor_base() noexcept(std::is_nothrow_default_constructible_v<storage_union<Ts...>>) = default;
  constexpr explicit variant_dtor_base(bool) noexcept: storage(false) {};

  constexpr variant_dtor_base(variant_dtor_base const& other) = default;
  constexpr variant_dtor_base(variant_dtor_base&& other) noexcept = default;

  constexpr variant_dtor_base& operator=(variant_dtor_base const&) = default;
  constexpr variant_dtor_base& operator=(variant_dtor_base&&) noexcept(((
      std::is_nothrow_move_constructible_v<Ts> && std::is_nothrow_move_assignable_v<Ts>) && ...)) = default;

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

  storage_union<Ts...> storage;
  size_t index_ = 0;
};

template<typename... Ts>
struct variant_dtor_base<false, Ts...> {
  constexpr variant_dtor_base() noexcept(std::is_nothrow_default_constructible_v<storage_union<Ts...>>) = default;
  constexpr explicit variant_dtor_base(bool) noexcept: storage(false) {};

  constexpr variant_dtor_base(variant_dtor_base const& other) = default;
  constexpr variant_dtor_base(variant_dtor_base&& other) noexcept = default;

  constexpr variant_dtor_base& operator=(variant_dtor_base const&) = default;
  constexpr variant_dtor_base& operator=(variant_dtor_base&&) noexcept(((
      std::is_nothrow_move_constructible_v<Ts> && std::is_nothrow_move_assignable_v<Ts>) && ...)) = default;

  template<typename U, typename... Args>
  constexpr explicit variant_dtor_base(in_place_type_t<U> in_place_flag, Args&& ... args)
      : storage(in_place_flag, std::forward<Args>(args)...),
        index_(get_index_of_type_v<U, Ts...>) {}

  template<size_t I, typename... Args>
  constexpr explicit variant_dtor_base(in_place_index_t<I> in_place_flag, Args&& ... args)
      : storage(in_place_flag, std::forward<Args>(args)...),
        index_(I) {}

  ~variant_dtor_base() {
    // TODO: trivially: сделать базы
    if (index_ != variant_npos) {
      destroy_stg(index_);
    }
  }

  constexpr size_t index() const noexcept {
    return index_;
  }

  template<size_t... Is>
  void call_destroy_stg(std::index_sequence<Is...>, size_t index) {
    using dtype = void (*)(storage_union<Ts...>&);
    static constexpr dtype destroyers[] = {
        [](storage_union<Ts...>& stg) { stg.template destroy_stg<Is>(); }...
    };
    destroyers[index](storage);
  }

  constexpr void destroy_stg(size_t index) {
    call_destroy_stg(std::index_sequence_for<Ts...>{}, index);
  }
  storage_union<Ts...> storage;
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

template<typename Table>
constexpr auto get_stg_from_table(Table&& table) {
  return table;
}

template<typename Table, typename VariantStg, typename... VariantStgs>
constexpr auto get_stg_from_table(Table&& table, VariantStg&& stg, VariantStgs&& ... stgs) {
  return get_stg_from_table(table[stg.index()], std::forward<VariantStgs>(stgs)...);
}

/*
  std::invoke_result_t<Visitor,
                                                                variant_stg_alternative_t<0,
                                                                                          std::decay_t<VariantStorages>>...>
 */
template<typename Visitor, typename... VariantStorages>
constexpr void visit_stg(Visitor&& vis, VariantStorages&& ... variant_stgs) {
  get_stg_from_table(stg_table_impl<sizeof...(VariantStorages) == 1,
                                    void,
                                    Visitor, 0,
                                    decltype(variant_stgs.storage)...>::make_stg_table(std::index_sequence<>{},
                                                                                       variant_stg_indexes_t<std::decay_t<
                                                                                           get_nth_type_t<0,
                                                                                                          VariantStorages...>>
                                                                                       >{}),
                     variant_stgs...)(std::forward<Visitor>(vis),
                                      std::move(variant_stgs.storage)...);
}