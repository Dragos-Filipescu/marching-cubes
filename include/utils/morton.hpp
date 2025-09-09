#pragma once
#ifndef MARCHING_CUBES_UTILS_MORTON_HPP
#define MARCHING_CUBES_UTILS_MORTON_HPP

#include <concepts>
#include <cstdint>

#include <core/aliases.hpp>

namespace marching_cubes::utils::morton {

    namespace detail {

        //------------------------------------------------------------------------------
        // Generic Morton bit‑interleaving.
        // Supports arbitrary integer widths and strides (e.g., 2D or 3D interleaving)
        //------------------------------------------------------------------------------

        // Interleave the lowest `Bits` bits of `x` with `Stride-1` zero bits between each.
        // E.g., Stride=2 → x0 0 x1 0 x2 0 ...
        template<
            std::unsigned_integral T,
            std::size_t Bits,
            std::size_t Stride
        >
        constexpr T interleaveBits(T value) noexcept
        {
            T result = 0;
            for (std::size_t i = 0; i < Bits; ++i) {
                result |= ((value >> i) & T{ 1 }) << (i * Stride);
            }
            return result;
        }

        // Deinterleave: extract every `Stride`-th bit starting at offset `Offset`
        // E.g., Stride=2, Offset=0 extracts bits 0,2,4,... to reconstruct original.
        template<
            std::unsigned_integral T,
            std::size_t Bits,
            std::size_t Stride,
            std::size_t Offset
        >
        constexpr T deinterleaveBits(T value) noexcept
        {
            T result = 0;
            for (std::size_t i = 0; i < Bits; ++i) {
                result |= ((value >> (i * Stride + Offset)) & T{ 1 }) << i;
            }
            return result;
        }

        static constexpr std::size_t morton2DStride = 2;
        static constexpr std::size_t morton3DStride = 3;

        static constexpr std::size_t xOffset = 0;
        static constexpr std::size_t yOffset = 1;
        static constexpr std::size_t zOffset = 2;
    }

    //------------------------------------------------------------------------------
    // 2D Morton encoding/decoding for up to 32-bit x,y (fits into 64-bit code)
    //------------------------------------------------------------------------------

    static constexpr std::size_t Morton2DBits = 32ULL;
    using Morton2T = u64; // stores up to 64 bits of interleaved data
    using Morton2ComponentT = u32;

    constexpr Morton2T encode2D(Morton2ComponentT x, Morton2ComponentT y) noexcept
    {
        return (detail::interleaveBits<Morton2T, Morton2DBits, detail::morton2DStride>(x) << detail::xOffset)
            | (detail::interleaveBits<Morton2T, Morton2DBits, detail::morton2DStride>(y) << detail::yOffset);
    }

    constexpr Morton2ComponentT decode2D_x(Morton2T code) noexcept {
        return static_cast<Morton2ComponentT>(
            detail::deinterleaveBits<
                Morton2T,
                Morton2DBits,
                detail::morton2DStride,
                detail::xOffset
            >(code)
        );
    }

    constexpr Morton2ComponentT decode2D_y(Morton2T code) noexcept {
        return static_cast<Morton2ComponentT>(
            detail::deinterleaveBits<
                Morton2T,
                Morton2DBits,
                detail::morton2DStride,
                detail::yOffset
            >(code)
        );
    }

    //------------------------------------------------------------------------------
    // 3D Morton encoding/decoding for up to 21-bit x,y,z (fits into 64-bit code)
    //------------------------------------------------------------------------------

    static constexpr std::size_t Morton3DBits = 21ULL;
    using Morton3T = u64; // stores up to 63 bits of interleaved data
    using Morton3ComponentT = u32;

    constexpr Morton3T encode3D(Morton3ComponentT x, Morton3ComponentT y, Morton3ComponentT z) noexcept {
        return (detail::interleaveBits<Morton3T, Morton3DBits, detail::morton3DStride>(x) << detail::xOffset)
            | (detail::interleaveBits<Morton3T, Morton3DBits, detail::morton3DStride>(y) << detail::yOffset)
            | (detail::interleaveBits<Morton3T, Morton3DBits, detail::morton3DStride>(z) << detail::zOffset);
    }

    constexpr Morton3ComponentT decode3D_x(Morton3T code) noexcept {
        return static_cast<Morton3ComponentT>(
            detail::deinterleaveBits<
                Morton3T,
                Morton3DBits,
                detail::morton3DStride,
                detail::xOffset
            >(code)
        );
    }

    constexpr Morton3ComponentT decode3D_y(Morton3T code) noexcept {
        return static_cast<Morton3ComponentT>(
            detail::deinterleaveBits<
                Morton3T,
                Morton3DBits,
                detail::morton3DStride,
                detail::yOffset
            >(code)
        );
    }

    constexpr Morton3ComponentT decode3D_z(Morton3T code) noexcept {
        return static_cast<Morton3ComponentT>(
            detail::deinterleaveBits<
                Morton3T,
                Morton3DBits,
                detail::morton3DStride,
                detail::zOffset
            >(code)
        );
    }

	static_assert(decode2D_x(encode2D(0x12345678u, 0x9ABCDEF0u)) == 0x12345678u, "2D decode_x failed");
	static_assert(decode2D_y(encode2D(0x12345678u, 0x9ABCDEF0u)) == 0x9ABCDEF0u, "2D decode_y failed");

	static_assert(decode3D_x(encode3D(0x12345u, 0x9ABCDu, 0xDEADBu)) == 0x12345u, "3D decode_x failed");
	static_assert(decode3D_y(encode3D(0x12345u, 0x9ABCDu, 0xDEADBu)) == 0x9ABCDu, "3D decode_y failed");
	static_assert(decode3D_z(encode3D(0x12345u, 0x9ABCDu, 0xDEADBu)) == 0xDEADBu, "3D decode_z failed");

} // namespace marching_cubes::utils::morton

#endif // MARCHING_CUBES_UTILS_MORTON_HPP
