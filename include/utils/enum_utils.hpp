#pragma once
#ifndef MARCHING_CUBES_UTILS_ENUM_HPP
#define MARCHING_CUBES_UTILS_ENUM_HPP

#include <cstdint>
#include <ostream>
#include <type_traits>

namespace marching_cubes::utils::enum_utils {

    template<typename T>
    concept Enum = std::is_enum_v<T>;

    template<Enum E>
    [[nodiscard]] constexpr auto toUnderlying(E e) noexcept
    {
        return static_cast<std::underlying_type_t<E>>(e);
    }

    template<Enum E>
    std::ostream& operator<<(std::ostream& os, const E& e)
    {
        if constexpr (std::is_signed_v<std::underlying_type_t<E>>) {
            os << static_cast<std::intmax_t>(toUnderlying(e));
        }
        else {
            os << static_cast<std::uintmax_t>(toUnderlying(e));
        }
    };
}

#define INCLUDE_ENUM_OSTREAM \
    using ::marching_cubes::utils::enum_utils::operator<<;

#endif // !MARCHING_CUBES_UTILS_ENUM_HPP

