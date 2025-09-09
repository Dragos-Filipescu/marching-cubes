#pragma once
#ifndef MARCHING_CUBES_UTILS_BIT_ENUM_HPP
#define MARCHING_CUBES_UTILS_BIT_ENUM_HPP

#include <concepts>
#include <type_traits>

#include <utils/bit_utils.hpp>
#include <utils/enum_utils.hpp>

#define BIT_ENUM_TAG_MEMBER_NAME marching_cubes_utils_bit_enum_tag_

namespace marching_cubes::utils::bit_enum {

    using enum_utils::toUnderlying;

    /// Concept to restrict the bitwise operators only to registered bit enums.
    template<typename T>
    concept BitEnum = enum_utils::Enum<T>
		&& std::unsigned_integral<std::underlying_type_t<T>>
        && requires {
        { T::BIT_ENUM_TAG_MEMBER_NAME };
    };

    /// normal and transparent operator|

    template<BitEnum E>
    [[nodiscard]] constexpr E operator|(E lhs, E rhs) noexcept
    {
        return static_cast<E>(toUnderlying(lhs) | toUnderlying(rhs));
    }

    template<BitEnum E, typename U>
        requires std::convertible_to<U, std::underlying_type_t<E>>
    [[nodiscard]] constexpr E operator|(E lhs, U rhs) noexcept {
        using T = std::underlying_type_t<E>;
        return static_cast<E>(toUnderlying(lhs) | static_cast<T>(rhs));
    }

    template<BitEnum E, typename U>
        requires std::convertible_to<U, std::underlying_type_t<E>>
    [[nodiscard]] constexpr E operator|(U lhs, E rhs) noexcept {
        using T = std::underlying_type_t<E>;
        return static_cast<E>(static_cast<T>(lhs) | toUnderlying(rhs));
    }

    /// normal and transparent operator&

    template<BitEnum E>
    [[nodiscard]] constexpr E operator&(E lhs, E rhs) noexcept
    {
        return static_cast<E>(toUnderlying(lhs) & toUnderlying(rhs));
    }

    template<BitEnum E, typename U>
        requires std::convertible_to<U, std::underlying_type_t<E>>
    [[nodiscard]] constexpr E operator&(E lhs, U rhs) noexcept {
        using T = std::underlying_type_t<E>;
        return static_cast<E>(toUnderlying(lhs) & static_cast<T>(rhs));
    }

    template<BitEnum E, typename U>
        requires std::convertible_to<U, std::underlying_type_t<E>>
    [[nodiscard]] constexpr E operator&(U lhs, E rhs) noexcept {
        using T = std::underlying_type_t<E>;
        return static_cast<E>(static_cast<T>(lhs) & toUnderlying(rhs));
    }

    /// normal and transparent operator^

    template<BitEnum E>
    [[nodiscard]] constexpr E operator^(E lhs, E rhs) noexcept
    {
        return static_cast<E>(toUnderlying(lhs) ^ toUnderlying(rhs));
    }

    template<BitEnum E, typename U>
        requires std::convertible_to<U, std::underlying_type_t<E>>
    [[nodiscard]] constexpr E operator^(E lhs, U rhs) noexcept {
        using T = std::underlying_type_t<E>;
        return static_cast<E>(toUnderlying(lhs) ^ static_cast<T>(rhs));
    }

    template<BitEnum E, typename U>
        requires std::convertible_to<U, std::underlying_type_t<E>>
    [[nodiscard]] constexpr E operator^(U lhs, E rhs) noexcept {
        using T = std::underlying_type_t<E>;
        return static_cast<E>(static_cast<T>(lhs) ^ toUnderlying(rhs));
    }

    /// operator~

    template<BitEnum E>
    [[nodiscard]] constexpr E operator~(E e) noexcept
    {
        return static_cast<E>(~toUnderlying(e));
    }

    /// operator<< and operator>>

    template<BitEnum E>
    [[nodiscard]] constexpr E operator<<(E e, unsigned int shift) noexcept {
        return static_cast<E>(toUnderlying(e) << shift);
    }

    template<BitEnum E>
    [[nodiscard]] constexpr E operator>>(E e, unsigned int shift) noexcept {
        return static_cast<E>(toUnderlying(e) >> shift);
    }

    /// normal and transparent operator|=

    template<BitEnum E>
    constexpr E& operator|=(E& lhs, E rhs) noexcept {
        lhs = lhs | rhs;
        return lhs;
    }

    template<BitEnum E, typename U>
        requires std::convertible_to<U, std::underlying_type_t<E>>
    constexpr E& operator|=(E& lhs, U rhs) noexcept {
        lhs = lhs | rhs;
        return lhs;
    }

    /// normal and transparent operator&=

    template<BitEnum E>
    constexpr E& operator&=(E& lhs, E rhs) noexcept {
        lhs = lhs & rhs;
        return lhs;
    }

    template<BitEnum E, typename U>
        requires std::convertible_to<U, std::underlying_type_t<E>>
    constexpr E& operator&=(E& lhs, U rhs) noexcept {
        lhs = lhs & rhs;
        return lhs;
    }

    /// normal and transparent operator^=

    template<BitEnum E>
    constexpr E& operator^=(E& lhs, E rhs) noexcept {
        lhs = lhs ^ rhs;
        return lhs;
    }

    template<BitEnum E, typename U>
        requires std::convertible_to<U, std::underlying_type_t<E>>
    constexpr E& operator^=(E& lhs, U rhs) noexcept {
        lhs = lhs ^ rhs;
        return lhs;
    }

    /// operator<<= and operator>>=

    template<BitEnum E>
    constexpr E& operator<<=(E& e, unsigned int shift) noexcept {
        e = static_cast<E>(toUnderlying(e) << shift);
        return e;
    }

    template<BitEnum E>
    constexpr E& operator>>=(E& e, unsigned int shift) noexcept {
        e = static_cast<E>(toUnderlying(e) >> shift);
        return e;
    }

    /// normal and transparent operator<=>

    template<BitEnum E>
    [[nodiscard]] constexpr auto operator<=>(E e1, E e2) noexcept
    {
        return toUnderlying(e1) <=> toUnderlying(e2);
    }

    template<BitEnum E, typename U>
        requires std::convertible_to<U, std::underlying_type_t<E>>
    [[nodiscard]] constexpr auto operator<=>(E e, U u) noexcept
    {
        using T = std::underlying_type_t<E>;
        return toUnderlying(e) <=> static_cast<T>(u);
    }

    template<BitEnum E, typename U>
        requires std::convertible_to<U, std::underlying_type_t<E>>
    [[nodiscard]] constexpr auto operator<=>(U u, E e) noexcept
    {
        using T = std::underlying_type_t<E>;
        return static_cast<T>(u) <=> toUnderlying(e);
    }

    /// transparent operator==

    /// normal operator== is already provided implicitly by c++

    template<BitEnum E, typename U>
        requires std::convertible_to<U, std::underlying_type_t<E>>
    [[nodiscard]] constexpr bool operator==(E e, U u) noexcept
    {
        using T = std::underlying_type_t<E>;
        return toUnderlying(e) == static_cast<T>(u);
    }

    template<BitEnum E, typename U>
        requires std::convertible_to<U, std::underlying_type_t<E>>
    [[nodiscard]] constexpr bool operator==(U u, E e) noexcept
    {
        using T = std::underlying_type_t<E>;
        return static_cast<T>(u) == toUnderlying(e);
    }

    template<BitEnum E>
    [[nodiscard]] constexpr bool any(E e) noexcept
    {
		return bit_utils::any(toUnderlying(e));
    }

    template<BitEnum E>
    [[nodiscard]] constexpr bool none(E e) noexcept
    {
        return bit_utils::none(toUnderlying(e));
    }

    template<BitEnum E>
    [[nodiscard]] constexpr bool hasBit(E e, std::size_t bitIdx) noexcept
    {
        return bit_utils::hasBit(toUnderlying(e), bitIdx);
    }

    template<BitEnum E>
    [[nodiscard]] constexpr std::underlying_type_t<E> bit(E e, std::size_t bitIdx) noexcept
    {
		return bit_utils::bit(toUnderlying(e), bitIdx);
    }


    template<BitEnum E>
    [[nodiscard]] constexpr E setBit(E e, std::size_t bitIdx) noexcept
    {
        return E{ bit_utils::setBit(toUnderlying(e), bitIdx) };
    }

    template<BitEnum E, std::unsigned_integral ...BitIdxs>
    [[nodiscard]] constexpr E setBits(E e, BitIdxs... bitIdxs) noexcept
    {
        return E{ bit_utils::setBits(toUnderlying(e), bitIdxs) };
    }

    template<BitEnum E>
    [[nodiscard]] constexpr E toggleBit(E e, std::size_t bitIdx) noexcept
    {
        return E{ bit_utils::toggleBit(toUnderlying(e), bitIdx) };
    }
}

#define BIT_ENUM_TAG BIT_ENUM_TAG_MEMBER_NAME = 0

#define INCLUDE_BIT_ENUM_BIT_OPS                            \
    using ::marching_cubes::utils::bit_enum::operator|;     \
    using ::marching_cubes::utils::bit_enum::operator&;     \
    using ::marching_cubes::utils::bit_enum::operator^;     \
    using ::marching_cubes::utils::bit_enum::operator~;     \
    using ::marching_cubes::utils::bit_enum::operator|=;    \
    using ::marching_cubes::utils::bit_enum::operator&=;    \
    using ::marching_cubes::utils::bit_enum::operator^=;

#define INCLUDE_BIT_ENUM_BIT_SHIFT                          \
    using ::marching_cubes::utils::bit_enum::operator<<;    \
    using ::marching_cubes::utils::bit_enum::operator>>;    \
    using ::marching_cubes::utils::bit_enum::operator<<=;   \
    using ::marching_cubes::utils::bit_enum::operator>>=;

#define INCLUDE_BIT_ENUM_COMPARISON                         \
    using ::marching_cubes::utils::bit_enum::operator<=>;   \
    using ::marching_cubes::utils::bit_enum::operator==;

#define INCLUDE_BIT_ENUM_ALL    \
    INCLUDE_BIT_ENUM_BIT_OPS    \
    INCLUDE_BIT_ENUM_BIT_SHIFT  \
    INCLUDE_BIT_ENUM_COMPARISON

#endif // !MARCHING_CUBES_UTILS_BIT_ENUM_HPP

