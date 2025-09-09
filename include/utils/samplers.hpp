#pragma once
#ifndef MARCHING_CUBES_UTILS_SDFS_HPP
#define MARCHING_CUBES_UTILS_SDFS_HPP

#include <glm/glm.hpp>
#include <cmath>
#include <core/aliases.hpp>

#include <utils/marching_cubes.hpp>

namespace marching_cubes::utils::samplers {

    struct Sphere {
        glm::vec3 c; // center
        f32 r;
        f32 operator()(f32 x, f32 y, f32 z) const noexcept
        {
            return glm::length(glm::vec3{ x, y, z } - c) - r;
        }
    };

    struct Box {
        glm::vec3 c; // center
        glm::vec3 b; // half extents
        f32 operator()(f32 x, f32 y, f32 z) const noexcept
        {
            glm::vec3 q = glm::abs(glm::vec3{ x, y, z } - c) - b;
            return glm::length(glm::max(q, glm::vec3{ 0.0f })) +
                glm::min(glm::max(q.x, glm::max(q.y, q.z)), 0.0f);
        }
    };

    struct Torus {
        glm::vec3 c; // center
        f32 R;
        f32 r;
        f32 operator()(f32 x, f32 y, f32 z) const noexcept
        {
            glm::vec3 p = glm::vec3{ x, y, z } - c;
            glm::vec2 q{ glm::length(glm::vec2{ p.x, p.z }) - R, p.y };
            return glm::length(q) - r;
        }
    };

    struct Cylinder {
        glm::vec3 c; // center
        f32 r;
        f32 h;
        f32 operator()(f32 x, f32 y, f32 z) const noexcept
        {
            glm::vec3 p = glm::vec3{ x, y, z } - c;
            glm::vec2 d = glm::abs(glm::vec2{ glm::length(glm::vec2{ p.x, p.z }), p.y }) - glm::vec2{ r, h };
            return glm::min(glm::max(d.x, d.y), 0.0f) + glm::length(glm::max(d, 0.0f));
        }
    };

    struct Cone {
        glm::vec3 c; // apex base centerline
        f32 h, r1, r2;
        f32 operator()(f32 x, f32 y, f32 z) const noexcept
        {
            glm::vec3 p = glm::vec3{ x, y, z } - c;
            f32 q = glm::length(glm::vec2{ p.x, p.z });
            f32 r = glm::mix(r1, r2, p.y / h);
            return glm::max(glm::length(glm::vec2{ q, p.y }) - h, q - r);
        }
    };

    struct Capsule {
        glm::vec3 a, b; // segment endpoints
        f32 r;
        f32 operator()(f32 x, f32 y, f32 z) const noexcept
        {
            glm::vec3 p{ x, y, z };
            glm::vec3 pa = p - a, ba = b - a;
            f32 h = glm::clamp(glm::dot(pa, ba) / glm::dot(ba, ba), 0.0f, 1.0f);
            return glm::length(pa - ba * h) - r;
        }
    };


    struct Gyroid {
        glm::vec3 offset;
        f32 scale;
        f32 operator()(f32 x, f32 y, f32 z) const noexcept
        {
            glm::vec3 p = (glm::vec3{ x, y, z } - offset) * scale;
            return std::sin(p.x) * std::cos(p.y)
                + std::sin(p.y) * std::cos(p.z)
                + std::sin(p.z) * std::cos(p.x);
        }
    };

    struct TerrainNoiseSampler {
        f32 frequency = 0.003f; // broader scale
        f32 amplitude = 40.0f;  // vertical variation
        f32 baseHeight = 10.0f; // average terrain level
        f32 ridgeHeight = 0.5f; // extra shaping factor

        f32 fBm2D(
            glm::vec2 p,
            f32 frequency,
            f32 amplitude,
            f32 baseHeight,
            std::size_t octaves = 5
        ) const
        {
            f32 n = 0.0, amp = 1.0, freq = frequency;
            f32 sumAmp = 0.0;
            for (std::size_t i = 0; i < octaves; i++) {
                // Use ridged noise to avoid "blobby" terrain
                f32 v = glm::simplex(p * freq);
                v = 1.0f - fabs(v); // ridged
                n += v * amp;
                sumAmp += amp;
                freq *= 2.0f;
                amp *= 0.5f;
            }
            return baseHeight + amplitude * (n / sumAmp);
        }

        f32 operator()(f32 x, f32 y, f32 z) const noexcept
        {
            // Terrain height at (x, z)
            f32 h = fBm2D(glm::vec2{ x, z }, frequency, amplitude, baseHeight);

            // Signed distance field:
            // Positive = above terrain, negative = below
            return y - h;
        }
    };

} // namespace marching_cubes::utils::sdfs

#endif // !MARCHING_CUBES_UTILS_SDFS_HPP
