#pragma once
#include "storage_union.h"
#include "variadic_utils.h"

template<typename T>
using value_holder_zero = value_holder_index<0>;

template<size_t ind, typename base>
constexpr decltype(auto) get(base&& v) {
  return std::forward<base>(v).storage.template get_stg<ind>();
}

template<typename Table>
constexpr auto const& get_from_table(Table const& table) {
  return table;
}

template<typename Table, typename... Is>
constexpr auto const& get_from_table(Table const& table, size_t index, Is... indexes) {
  return get_from_table(table[index], indexes...);
}

template<bool indexed, typename R, typename Visitor, size_t CurrentLvl,
    typename PrefixSeq, typename VariantSeq, typename... Variants>
struct table_cache;

template<typename R, typename Visitor, size_t CurrentLvl, size_t... Prefix, typename... Variants>
struct table_cache<false, R, Visitor, CurrentLvl,
                   std::index_sequence<Prefix...>, std::index_sequence<>, Variants...> {
  static constexpr auto array = [](Visitor vis, Variants... vars) {
    return vis(get<Prefix>(std::forward<Variants>(vars))...);
  };
};

template<typename R, typename Visitor, size_t CurrentLvl, size_t... Prefix, typename... Variants>
struct table_cache<true, R, Visitor, CurrentLvl,
                   std::index_sequence<Prefix...>, std::index_sequence<>, Variants...> {
  static constexpr auto array = [](Visitor vis, Variants... vars) {
    return vis(get<Prefix>(std::forward<Variants>(vars))..., value_holder_index<Prefix>{}...);
  };
};

template<bool indexed, typename R, typename Visitor, size_t CurrentLvl,
    size_t... Prefix, size_t... VariantIndexes, typename... Variants>
struct table_cache<indexed, R, Visitor, CurrentLvl,
                   std::index_sequence<Prefix...>, std::index_sequence<VariantIndexes...>, Variants...> {
  static constexpr auto array = std::experimental::make_array(table_cache<indexed, R, Visitor, CurrentLvl + 1,
                                                                          std::index_sequence<Prefix...,
                                                                                              VariantIndexes>,
                                                                          alt_indexes_by_ind_t<CurrentLvl + 1,
                                                                                               Variants...>,
                                                                          Variants...>::array...);
};

template<bool indexed, typename R, typename Visitor, typename... Variants>
using table_cache_t = table_cache<indexed, R, Visitor, 0,
                                  std::index_sequence<>, alt_indexes_t<types_at_t<0, Variants...>>,
                                  Variants...>;

template<typename Visitor, typename... Variants>
constexpr decltype(auto) visit_indexed(Visitor&& vis, Variants&& ... vars) {
  return get_from_table(table_cache_t<true,
                                      std::invoke_result_t<Visitor,
                                                           alternative_t<0, Variants>...,
                                                           value_holder_zero<Variants>...>,
                                      Visitor&&, Variants&& ...>::array,
                        vars.index()...)(std::forward<Visitor>(vis),
                                         std::forward<Variants>(vars)...);
}
