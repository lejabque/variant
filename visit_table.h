#pragma once
#include "variadic_utils.h"

namespace variant_utils {
template<size_t ind, typename base>
constexpr decltype(auto) get_impl(base&& v) {
  return std::forward<base>(v).storage.template get_stg<ind>();
}

template<typename T>
using value_holder_zero = std::integral_constant<size_t, 0>;

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
struct table_cache_impl;

template<typename R, typename Visitor, size_t CurrentLvl, size_t... Prefix, typename... Variants>
struct table_cache_impl<false, R, Visitor, CurrentLvl,
                        std::index_sequence<Prefix...>, std::index_sequence<>, Variants...> {
  static constexpr auto array() {
    return [](Visitor vis, Variants... vars) {
      return vis(get_impl<Prefix>(static_cast<Variants>(vars))...);
    };
  }
};

template<typename R, typename Visitor, size_t CurrentLvl, size_t... Prefix, typename... Variants>
struct table_cache_impl<true, R, Visitor, CurrentLvl,
                        std::index_sequence<Prefix...>, std::index_sequence<>, Variants...> {
  static constexpr auto array() {
    return [](Visitor vis, Variants... vars) {
      return vis(get_impl<Prefix>(static_cast<Variants>(vars))..., std::integral_constant<size_t, Prefix>{}...);
    };
  }
};

template<class... Args>
constexpr std::array<typename std::common_type_t<Args...>, sizeof...(Args)> make_array(Args&& ... args) {
  return {std::forward<Args>(args)...};
}

template<bool indexed, typename R, typename Visitor, size_t CurrentLvl,
    size_t... Prefix, size_t... VariantIndexes, typename... Variants>
struct table_cache_impl<indexed, R, Visitor, CurrentLvl,
                        std::index_sequence<Prefix...>, std::index_sequence<VariantIndexes...>, Variants...> {
  static constexpr auto array() {
    return make_array(table_cache_impl<indexed, R, Visitor, CurrentLvl + 1,
                                       std::index_sequence<Prefix...,
                                                           VariantIndexes>,
                                       variant_utils::alt_indexes_by_ind_t<CurrentLvl + 1,
                                                                           Variants...>,
                                       Variants...>::array()...);
  }
};

template<bool indexed, typename R, typename Visitor, typename... Variants>
using table_cache_t = table_cache_impl<indexed, R, Visitor, 0,
                                       std::index_sequence<>, alt_indexes_t<types_at_t<0, Variants...>>,
                                       Variants...>;


template<bool indexed, typename Visitor, typename... Variants>
struct visit_table;

template<typename Visitor, typename... Variants>
struct visit_table<true, Visitor, Variants...> {
  static constexpr auto array = table_cache_t<true,
                                              std::invoke_result_t<Visitor,
                                                                   alternative_t<0, Variants&&>...,
                                                                   value_holder_zero<Variants>...>,
                                              Visitor&&, Variants&& ...>::array();
};

template<typename Visitor, typename... Variants>
struct visit_table<false, Visitor, Variants...> {
  static constexpr auto array = table_cache_t<false,
                                              std::invoke_result_t<Visitor,
                                                                   alternative_t<0, Variants&&>...>,
                                              Visitor&&, Variants&& ...>::array();
};

template<typename Visitor, typename... Variants>
constexpr decltype(auto) visit_indexed(Visitor&& vis, Variants&& ... vars) {
  return get_from_table(visit_table<true, Visitor&&, Variants&& ...>::array,
                        vars.index()...)(std::forward<Visitor>(vis),
                                         std::forward<Variants>(vars)...);
}
} // namespace variant_utils
