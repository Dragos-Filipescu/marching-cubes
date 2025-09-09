#pragma once
#ifndef MARCHING_CUBES_UTILS_TYPE_SAFETY_HPP
#define MARCHING_CUBES_UTILS_TYPE_SAFETY_HPP

#include <concepts>

namespace marching_cubes::utils {

    namespace detail {

        template<typename Tag, typename T>
            requires std::integral<T> || std::floating_point<T>
        struct StrongNumber final {
            using Type = T;

            T value{};

            constexpr StrongNumber() noexcept = default;

            constexpr explicit StrongNumber(T v) noexcept
                : value{ v }
            {
            }

            constexpr operator T() const noexcept
            {
                return value;
            }
        };
    }

    template<typename Tag, std::integral T>
    using StrongIntegral = detail::StrongNumber<Tag, T>;

    template<typename Tag, std::floating_point T>
    using StrongFloat = detail::StrongNumber<Tag, T>;
}

#endif // !MARCHING_CUBES_UTILS_TYPE_SAFETY_HPP
