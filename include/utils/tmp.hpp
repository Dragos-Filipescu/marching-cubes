#pragma once
#ifndef MARCHING_CUBES_UTILS_TMP_HPP
#define MARCHING_CUBES_UTILS_TMP_HPP

#include <compare>
#include <concepts>
#include <cstdint>
#include <tuple>

#include <core/aliases.hpp>

namespace marching_cubes::utils::tmp {

    namespace detail {
        template<std::size_t I, typename T, typename... Ts>
        consteval std::size_t indexOfImpl()
        {
            if constexpr (I >= sizeof...(Ts)) {
                static_assert(false, "Attribute not found in pack.");
                return 0;
            }
            else if constexpr (std::same_as<T, std::tuple_element_t<I, std::tuple<Ts...>>>) {
                return I;
            }
            else {
                return indexOfImpl<I + 1, T, Ts...>();
            }
        }
    }

    template<typename T, typename... Ts>
    consteval std::size_t indexOf()
    {
        return detail::indexOfImpl<0, T, Ts...>();
    }

    template<typename T, typename... Ts>
    inline constexpr bool isOneOf = (std::same_as<T, Ts> || ...);

    struct Empty final {};

    template<auto T, auto Min, auto Max>
        requires (
            std::three_way_comparable_with<
                decltype(T),
                decltype(Min),
                std::strong_ordering
            >
            && std::three_way_comparable_with<
                decltype(T),
                decltype(Max),
                std::strong_ordering
            >
        )
    constexpr bool between = (Min <=> T && T <=> Max);
}
#endif // !MARCHING_CUBES_UTILS_TMP_HPP