#pragma once
#ifndef MARCHING_CUBES_UTILS_BIT_HPP
#define MARCHING_CUBES_UTILS_BIT_HPP

#include <bit>
#include <concepts>
#include <initializer_list>
#include <limits>
#include <type_traits>

namespace marching_cubes::utils::bit_utils {

    template<std::integral T>
    [[nodiscard]] constexpr bool hasBit(T value, std::size_t bitIdx) noexcept
    {
        using U = std::make_unsigned_t<T>;
        return (static_cast<U>(value) & (U{ 1 } << bitIdx)) != 0;
    }

    template<std::integral T>
    [[nodiscard]] constexpr T bit(T value, std::size_t bitIdx) noexcept
    {
        return static_cast<T>(hasBit(value, bitIdx));
    }

    template<std::integral T>
    [[nodiscard]] constexpr T setBit(T value, std::size_t bitIdx) noexcept
    {
        using U = std::make_unsigned_t<T>;
        constexpr std::size_t bitWidth = std::numeric_limits<U>::digits;
        if (bitIdx < bitWidth) {
            return static_cast<T>(static_cast<U>(value) | (U{ 1 } << bitIdx));
        }
        return value;
    }

    template<std::integral T, std::unsigned_integral ...BitIdxs>
    [[nodiscard]] constexpr T setBits(T value = T{ 0 }, BitIdxs... bitIdxs) noexcept
    {
        if constexpr (sizeof...(bitIdxs) == std::size_t{ 0 }) {
            return value;
        }
        else {
            using U = std::make_unsigned_t<T>;
            constexpr std::size_t bitWidth = std::numeric_limits<U>::digits;
            const U mask = ((U{ 1 } << static_cast<std::size_t>(bitIdxs)) | ...);
            return static_cast<T>(static_cast<U>(value) | mask);
        }
    }

    template<std::integral T>
    [[nodiscard]] constexpr T toggleBit(T value, std::size_t bitIdx) noexcept
    {
        using U = std::make_unsigned_t<T>;
        constexpr std::size_t bitWidth = std::numeric_limits<U>::digits;
        if (bitIdx < bitWidth) {
            return static_cast<T>(static_cast<U>(value) ^ (U{ 1 } << bitIdx));
        }
        return value;
    }

    template<std::integral T>
    [[nodiscard]] constexpr bool any(T value) noexcept
    {
        return std::popcount(value) != 0;
    }

    template<std::integral T>
    [[nodiscard]] constexpr bool any(T value, T mask) noexcept
    {
        return any(value & mask);
    }

    template<std::integral T>
    [[nodiscard]] constexpr bool none(T value) noexcept
    {
        return std::popcount(value) == 0;
    }

    template<std::integral T>
    [[nodiscard]] constexpr bool all(T value) noexcept
    {
        using U = std::make_unsigned_t<T>;
        return std::popcount(value) == std::numeric_limits<U>::digits;
    }

    template<std::integral T>
    [[nodiscard]] constexpr bool all(T value, T mask) noexcept
    {
        return (value & mask) == mask;
    }
}

#endif // !MARCHING_CUBES_UTILS_BIT_HPP

