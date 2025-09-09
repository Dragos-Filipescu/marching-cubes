#pragma once
#ifndef MARCHING_CUBES_UTILS_MARCHING_CUBES_HPP
#define MARCHING_CUBES_UTILS_MARCHING_CUBES_HPP

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <glm/gtx/hash.hpp>

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <concepts>
#include <functional>
#include <limits>
#include <list>
#include <ostream>
#include <print>
#include <stop_token>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <core/aliases.hpp>

#include <scene/vertex.hpp>

#include <utils/bit_enum.hpp>
#include <utils/bit_utils.hpp>
#include <utils/enum_utils.hpp>
#include <utils/render_conventions.hpp>
#include <utils/utils.hpp>

namespace marching_cubes::utils::marching_cubes {

    /*                          E4
                  4-----------------------------5
                 /|                            /|
                / |                           / |
             E7/  |                        E5/  |
              /   |                         /   |
             /    |E8                      /    |E9
            /     |         E6            /     |
           6-----------------------------7      |
           |      |                      |      |
           |      |                      |      |
           |      |             E0       |      |
           |      0----------------------|------1
           |     /                       |     /
        E11|    /                     E10|    /
           |   /E3                       |   /E1
           |  /                          |  /
           | /                           | /
           |/              E2            |/
           2-----------------------------3
    */

    INCLUDE_BIT_ENUM_BIT_OPS;
    INCLUDE_BIT_ENUM_COMPARISON;

    using enum_utils::toUnderlying;
    using render_conventions::RenderConventions;

    namespace detail {

        enum class Axis : u8 {
            X = 0,
            Y = 1,
            Z = 2,
        };

        enum class AxisMask : u8 {
            None = 0,
            X = u8{ 1 } << toUnderlying(Axis::X),
            Y = u8{ 1 } << toUnderlying(Axis::Y),
            Z = u8{ 1 } << toUnderlying(Axis::Z),
            All = (X | Y | Z),
            BIT_ENUM_TAG,
        };

        enum class Vertex : u8 {
            V0 = toUnderlying(AxisMask::None),
            V1 = toUnderlying(AxisMask::X),
            V2 = toUnderlying(AxisMask::Y),
            V3 = toUnderlying(AxisMask::X | AxisMask::Y),
            V4 = toUnderlying(AxisMask::Z),
            V5 = toUnderlying(AxisMask::X | AxisMask::Z),
            V6 = toUnderlying(AxisMask::Y | AxisMask::Z),
            V7 = toUnderlying(AxisMask::X | AxisMask::Y | AxisMask::Z),
        };

        enum class VertexMask : u8 {
            None = u8{ 0 },
            V0 = u8{ 1 } << toUnderlying(Vertex::V0),
            V1 = u8{ 1 } << toUnderlying(Vertex::V1),
            V2 = u8{ 1 } << toUnderlying(Vertex::V2),
            V3 = u8{ 1 } << toUnderlying(Vertex::V3),
            V4 = u8{ 1 } << toUnderlying(Vertex::V4),
            V5 = u8{ 1 } << toUnderlying(Vertex::V5),
            V6 = u8{ 1 } << toUnderlying(Vertex::V6),
            V7 = u8{ 1 } << toUnderlying(Vertex::V7),
            All = (V0 | V1 | V2 | V3 | V4 | V5 | V6 | V7),
            BIT_ENUM_TAG,
        };

        enum class Edge : u8 {
            E01 = u8{ 0 },    // Edge between corners 0 – 1
            E13 = u8{ 1 },    // Edge between corners 1 – 3
            E23 = u8{ 2 },    // Edge between corners 2 – 3
            E02 = u8{ 3 },    // Edge between corners 3 – 0
            E45 = u8{ 4 },    // Edge between corners 4 – 5
            E57 = u8{ 5 },    // Edge between corners 5 – 6
            E67 = u8{ 6 },    // Edge between corners 6 – 7
            E46 = u8{ 7 },    // Edge between corners 7 – 4
            E04 = u8{ 8 },    // Edge between corners 0 – 4
            E15 = u8{ 9 },    // Edge between corners 1 – 5
            E37 = u8{ 10 },   // Edge between corners 3 – 7
            E26 = u8{ 11 },   // Edge between corners 2 – 6
            ESp = u8{ 12 },   // Special value: extra vertex interior to the cube
            End = std::numeric_limits<u8>::max(),   // Stop condition, no more triangles
        };

        enum class EdgeMask : u16 {
            None = u16{ 0 },
            E01 = u16{ 1 } << toUnderlying(Edge::E01),   // Edge between corners 0 – 1
            E13 = u16{ 1 } << toUnderlying(Edge::E13),   // Edge between corners 1 – 3
            E23 = u16{ 1 } << toUnderlying(Edge::E23),   // Edge between corners 2 – 3
            E02 = u16{ 1 } << toUnderlying(Edge::E02),   // Edge between corners 3 – 0
            E45 = u16{ 1 } << toUnderlying(Edge::E45),   // Edge between corners 4 – 5
            E57 = u16{ 1 } << toUnderlying(Edge::E57),   // Edge between corners 5 – 6
            E67 = u16{ 1 } << toUnderlying(Edge::E67),   // Edge between corners 6 – 7
            E46 = u16{ 1 } << toUnderlying(Edge::E46),   // Edge between corners 7 – 4
            E04 = u16{ 1 } << toUnderlying(Edge::E04),   // Edge between corners 0 – 4
            E15 = u16{ 1 } << toUnderlying(Edge::E15),   // Edge between corners 1 – 5
            E37 = u16{ 1 } << toUnderlying(Edge::E37),   // Edge between corners 3 – 7
            E26 = u16{ 1 } << toUnderlying(Edge::E26),   // Edge between corners 2 – 6
            All = (E01 | E13 | E23 | E02 | E45 | E57 | E67 | E46 | E04 | E15 | E37 | E26),
            BIT_ENUM_TAG,
        };

        enum class AxisFlip : i8 {
            None = +1,  // no flip
            Flip = -1,  // flip the axis
        };

        enum class Face : u8 {
            XPos = 0u,
            XNeg,
            YPos,
            YNeg,
            ZPos,
            ZNeg,
        };

        enum class AmbiguityMask : u8 {
            None = u8{ 0 },
            XPos = u8{ 1 } << toUnderlying(Face::XPos),
            XNeg = u8{ 1 } << toUnderlying(Face::XNeg),
            YPos = u8{ 1 } << toUnderlying(Face::YPos),
            YNeg = u8{ 1 } << toUnderlying(Face::YNeg),
            ZPos = u8{ 1 } << toUnderlying(Face::ZPos),
            ZNeg = u8{ 1 } << toUnderlying(Face::ZNeg),
            Interior = ZNeg << 1u,
            All = (XPos | XNeg | YPos | YNeg | ZPos | ZNeg | Interior),
            BIT_ENUM_TAG,
        };

        constexpr std::size_t kCubeVertexCount = 8;
        constexpr std::size_t kCubeEdgeCount = 12;
        constexpr std::size_t kCubeFaceCount = 6;
        constexpr std::size_t kCubeFaceVertexCount = 4;
        constexpr std::size_t kCubeVertexStateCount = 1 << kCubeVertexCount;
        constexpr std::size_t kSpecialEdgeIdx = 12;
		constexpr std::size_t kAmbiguityInteriorIdx = 6;

        template<std::size_t TriangleCount>
        constexpr std::size_t kTriangleVertices = TriangleCount * 3ULL;

        using FaceQuad = std::array<Vertex, kCubeFaceVertexCount>;

        constexpr std::array<FaceQuad, kCubeFaceCount> kCubeFaceQuads = {
            {
                /* +X */ {{ Vertex::V1, Vertex::V3, Vertex::V5, Vertex::V7 }},
                /* -X */ {{ Vertex::V0, Vertex::V2, Vertex::V4, Vertex::V6 }},
                /* +Y */ {{ Vertex::V2, Vertex::V3, Vertex::V6, Vertex::V7 }},
                /* -Y */ {{ Vertex::V0, Vertex::V1, Vertex::V4, Vertex::V5 }},
                /* +Z */ {{ Vertex::V4, Vertex::V5, Vertex::V6, Vertex::V7 }},
                /* -Z */ {{ Vertex::V0, Vertex::V1, Vertex::V2, Vertex::V3 }},
            }
        };

        using MC33Variant = std::array<Edge, kTriangleVertices<12> + 1>;

        struct MC33Case final {
            std::size_t caseIdx{ 0 };
            // Fețele ambigue ale cubului (maxim 6), fiecare descrisă printr-un quad
            std::array<FaceQuad, kCubeFaceCount> ambiguousFaces{ kCubeFaceQuads };
            // Pentru acest caz canonic:
            // - ce fețe necesită un test de saddle, și dacă este necesar un test interior
            AmbiguityMask mask{ AmbiguityMask::None };
            // Masca de vertecși activați pentru acest caz
            VertexMask vertexMask{ VertexMask::None };
            // Indexul de rotație al acestui caz
            std::size_t rotIdx{ 0 };
            // Indică dacă masca este complementată (caz inversat)
            bool complemented{ false };
            // Triangulările pentru fiecare combinație posibilă de biți testați.
            // Pot exista maximum 6 variante (cazul 13)
            std::array<MC33Variant, 6> variants{};
            // Numărul de variante valide pentru acest caz
            u8 variantCount{ 0 };
            // Tabel de lookup: mapează AmbiguityMask -> index de variantă.
            // Dimensiunea maximă este 128, deoarece AmbiguityMask are 7 biți.
            std::array<u8, toUnderlying(AmbiguityMask::All) + 1> variantLUT{};
            constexpr bool operator==(const MC33Case&) const noexcept = default;
        };

        constexpr std::array<
            std::pair<
                Vertex,
                Vertex
            >,
            kCubeEdgeCount
        > kCubeEdges{
            {
                /// Muchiile feței de jos
                { Vertex::V0, Vertex::V1 }, // E01 (00)
                { Vertex::V1, Vertex::V3 }, // E13 (01)
                { Vertex::V2, Vertex::V3 }, // E23 (02)
                { Vertex::V0, Vertex::V2 }, // E02 (03)

                /// Muchiile feței de jos
                { Vertex::V4, Vertex::V5 }, // E45 (04)
                { Vertex::V5, Vertex::V7 }, // E57 (05)
                { Vertex::V6, Vertex::V7 }, // E67 (06)
                { Vertex::V4, Vertex::V6 }, // E46 (07)

                /// Muchiile dintre cele două fețe
                { Vertex::V0, Vertex::V4 }, // E04 (08)
                { Vertex::V1, Vertex::V5 }, // E15 (09)
                { Vertex::V3, Vertex::V7 }, // E37 (10)
                { Vertex::V2, Vertex::V6 }, // E26 (11)
            }
        };

        consteval auto makeEdgeTable()
        {
            constexpr std::size_t N = 256; // 2^8 possible corner states (inside/outside)

            constexpr auto buildMask = [&](std::size_t cubeConfig) noexcept {
                EdgeMask mask = EdgeMask::None;
                // for each of the 12 edges...
                for (std::size_t e = 0; e < kCubeEdges.size(); ++e) {
                    const auto [v1, v2] = kCubeEdges[e];
                    // extract the "inside/outside" bits for the two corners
                    const bool inside1 = bit_utils::hasBit(cubeConfig, toUnderlying(v1));
                    const bool inside2 = bit_utils::hasBit(cubeConfig, toUnderlying(v2));
                    // if they differ, the surface crosses this edge
                    if (inside1 != inside2) {
                        mask = bit_enum::setBit(mask, e);
                    }
                }
                return mask;
            };

            return[&buildMask]<std::size_t ...cubeConfig>(std::index_sequence<cubeConfig...> seq) {
                return std::array<EdgeMask, decltype(seq)::size()>{ buildMask(cubeConfig)... };
            }(std::make_index_sequence<N>{});
        }

        constexpr auto kEdgeTable = makeEdgeTable();

        // index of a vertex given by the coordinates, encoded as 3 bits
        constexpr Vertex indexOf(bool x, bool y, bool z) noexcept
        {
            return Vertex(
                (static_cast<std::size_t>(x) << toUnderlying(Axis::X))
                | (static_cast<std::size_t>(y) << toUnderlying(Axis::Y))
                | (static_cast<std::size_t>(z) << toUnderlying(Axis::Z))
            );
        }

        consteval auto makeAxisPermutations()
        {
            // start in lexicographical order:
            std::array<Axis, 3> axes{
                {
                    Axis::X,
                    Axis::Y,
                    Axis::Z,
                }
            };
            // we'll collect 6 permutations:
            std::array<std::array<Axis, 3>, 6> permutations{};
            std::size_t i = 0;

            do {
                permutations[i++] = axes;
            } while (
                std::next_permutation(
                    axes.begin(), axes.end(),
                    [](const Axis& lhs, const Axis& rhs) {
                        return toUnderlying(lhs)
                            < toUnderlying(rhs);
                    }
                )
                );

            return permutations;
        }

        constexpr auto permutationParity(const std::array<Axis, 3>& permutation) noexcept
        {
            enum class PermutationParity : i8 {
                Even = +1,
                Odd = -1,
            };

            i8 invertedAxes = 0;
            for (std::size_t i = 0; i < permutation.size(); ++i) {
                for (std::size_t j = i + 1; j < permutation.size(); ++j) {
                    if (permutation[j] < permutation[i]) {
                        ++invertedAxes;
                    }
                }
            }
            return ((invertedAxes & 1) == 0 ? PermutationParity::Even : PermutationParity::Odd);
        }

        consteval auto makeVertexRotationsTable()
        {
            /// Each of the 6 permutations describes a rotation of the cube's axes, as such:
            /// {X, Y, Z} -> {X, Z, Y} means "rotate the cube until the old Y axis is now
            /// aligned with the Z axis, and the old Z axis is now aligned with the Y axis"
            constexpr auto axisPermutations = makeAxisPermutations();

            constexpr std::array<
                std::array<AxisFlip, 3>,
                8
            > flipPermutations{
                {
                    //  X axis flip ,   Y axis flip,    Z axis flip
                    {{  AxisFlip::None, AxisFlip::None, AxisFlip::None  }},
                    {{  AxisFlip::Flip, AxisFlip::None, AxisFlip::None  }},
                    {{  AxisFlip::None, AxisFlip::Flip, AxisFlip::None  }},
                    {{  AxisFlip::None, AxisFlip::None, AxisFlip::Flip  }},
                    {{  AxisFlip::Flip, AxisFlip::Flip, AxisFlip::None  }},
                    {{  AxisFlip::Flip, AxisFlip::None, AxisFlip::Flip  }},
                    {{  AxisFlip::None, AxisFlip::Flip, AxisFlip::Flip  }},
                    {{  AxisFlip::Flip, AxisFlip::Flip, AxisFlip::Flip  }},
                }
            };

            std::array<
                std::array<Vertex, kCubeVertexCount>,
                24
            > table{};
            std::size_t idx = 0;

            for (
                const auto [
                    xAxisPermutation,
                    yAxisPermutation,
                    zAxisPermutation
                ] : axisPermutations
                ) {
                auto parity = permutationParity(
                    std::array<Axis, 3>{
                        {
                            xAxisPermutation,
                                yAxisPermutation,
                                zAxisPermutation,
                        }
                }
                );
                for (
                    const auto [
                        xAxisFlip,
                        yAxisFlip,
                        zAxisFlip
                    ] : flipPermutations
                    ) {
                    auto determinant = toUnderlying(parity)
                        * toUnderlying(xAxisFlip)
                        * toUnderlying(yAxisFlip)
                        * toUnderlying(zAxisFlip);

                    if (determinant != 1) {
                        // skip this permutation, it’s not a valid rotation
                        continue;
                    }
                    // for each of the 8 source vertices...
                    for (u8 v = 0; v < kCubeVertexCount; ++v) {
                        // unpack into centered coords [-1, +1]
                        const std::array<const i8, 3> coords = {
                            {
                                bit_utils::hasBit(v, toUnderlying(Axis::X)) ? +1 : -1,
                                bit_utils::hasBit(v, toUnderlying(Axis::Y)) ? +1 : -1,
                                bit_utils::hasBit(v, toUnderlying(Axis::Z)) ? +1 : -1,
                            }
                        };
                        // apply axis‐perm + sign
                        auto newX = toUnderlying(xAxisFlip)
                            * coords[toUnderlying(xAxisPermutation)];
                        auto newY = toUnderlying(yAxisFlip)
                            * coords[toUnderlying(yAxisPermutation)];
                        auto newZ = toUnderlying(zAxisFlip)
                            * coords[toUnderlying(zAxisPermutation)];
                        // map back from [-1, +1] -> [0, 1]
                        bool xFinal = newX > 0;
                        bool yFinal = newY > 0;
                        bool zFinal = newZ > 0;
                        table[idx][v] = indexOf(xFinal, yFinal, zFinal);
                    }
                    idx++;
                }
            }

            return table;
        }

        constexpr auto kVertexRotationsTable = makeVertexRotationsTable();

        // Return Face enum from axis + sign (+1 => Pos, -1 => Neg)
        constexpr Face makeFace(Axis axis, int sign) {
            switch (axis) {
            case Axis::X: return (sign >= 0) ? Face::XPos : Face::XNeg;
            case Axis::Y: return (sign >= 0) ? Face::YPos : Face::YNeg;
            case Axis::Z: return (sign >= 0) ? Face::ZPos : Face::ZNeg;
            }
            return Face::XPos; // unreachable
        }

        // Build a 24 x 6 table: for each rotation, where do {+/-X,+/-Y,+/-Z} go?
        consteval auto makeFaceRotationsTable()
        {
            constexpr auto axisPermutations = makeAxisPermutations();

            // Same flip set and determinant filter as makeVertexRotationsTable()
            constexpr std::array<std::array<AxisFlip, 3>, 8> flipPermutations{
                {
                    //  X axis flip ,   Y axis flip,    Z axis flip
                    {{ AxisFlip::None, AxisFlip::None, AxisFlip::None }},
                    {{ AxisFlip::Flip, AxisFlip::None, AxisFlip::None }},
                    {{ AxisFlip::None, AxisFlip::Flip, AxisFlip::None }},
                    {{ AxisFlip::None, AxisFlip::None, AxisFlip::Flip }},
                    {{ AxisFlip::Flip, AxisFlip::Flip, AxisFlip::None }},
                    {{ AxisFlip::Flip, AxisFlip::None, AxisFlip::Flip }},
                    {{ AxisFlip::None, AxisFlip::Flip, AxisFlip::Flip }},
                    {{ AxisFlip::Flip, AxisFlip::Flip, AxisFlip::Flip }},
                }
            };

            // Table[rotation][face] -> rotated face
            std::array<std::array<Face, kCubeFaceCount>, 24> table{};
            std::size_t idx = 0;

            for (const auto [px, py, pz] : axisPermutations) {
                auto parity = permutationParity(std::array<Axis, 3>{px, py, pz});

                for (const auto [fx, fy, fz] : flipPermutations) {
                    // accept only proper rotations (determinant +1)
                    int det = toUnderlying(parity)
                        * toUnderlying(fx)
                        * toUnderlying(fy)
                        * toUnderlying(fz);
                    if (det != 1) {
                        continue;
                    }

                    // Map canonical axis -> new-axis index (inverse of {px,py,pz} “pull” mapping)
                    // newX = old[px], newY = old[py], newZ = old[pz]
                    // => old X ends up on new axis N where pN == X, etc.
                    auto newAxisOfOld = [&](Axis a) -> std::pair<Axis, int> {
                        if (px == a) return { Axis::X, toUnderlying(fx) };
                        if (py == a) return { Axis::Y, toUnderlying(fy) };
                        /*pz==a*/   return { Axis::Z, toUnderlying(fz) };
                        };

                    // Fill 6 faces in canonical order: +X,-X,+Y,-Y,+Z,-Z
                    auto& row = table[idx];

                    // +X / -X
                    {
                        auto [na, sgn] = newAxisOfOld(Axis::X);
                        row[toUnderlying(Face::XPos)] = makeFace(na, +1 * sgn);
                        row[toUnderlying(Face::XNeg)] = makeFace(na, -1 * sgn);
                    }
                    // +Y / -Y
                    {
                        auto [na, sgn] = newAxisOfOld(Axis::Y);
                        row[toUnderlying(Face::YPos)] = makeFace(na, +1 * sgn);
                        row[toUnderlying(Face::YNeg)] = makeFace(na, -1 * sgn);
                    }
                    // +Z / -Z
                    {
                        auto [na, sgn] = newAxisOfOld(Axis::Z);
                        row[toUnderlying(Face::ZPos)] = makeFace(na, +1 * sgn);
                        row[toUnderlying(Face::ZNeg)] = makeFace(na, -1 * sgn);
                    }

                    ++idx;
                }
            }

            return table;
        }

        constexpr auto kFaceRotationsTable = makeFaceRotationsTable();

        consteval auto rotatedVertexMask(VertexMask mask, std::size_t rotIdx)
        {
            // pull out each bit, send it to its permuted slot
            VertexMask out = VertexMask::None;
            for (std::size_t v = 0; v < kCubeVertexCount; ++v) {
                if (bit_enum::hasBit(mask, v)) {
                    out = bit_enum::setBit(out, toUnderlying(kVertexRotationsTable[rotIdx][v]));
                }
            }
            return out;
        }

        consteval auto rotatedVertex(Vertex v, std::size_t rotIdx)
        {
            return kVertexRotationsTable[rotIdx][toUnderlying(v)];
        }

        consteval auto makeCanonicalVertexMasks()
        {
            return std::array<VertexMask, 15> {
                {
                    VertexMask::None,                                                   // 0 - no corners
                    VertexMask::V2,                                                     // 1 - corner 2
                    VertexMask::V2 | VertexMask::V3,                                    // 2 - corners 2, 3
                    VertexMask::V2 | VertexMask::V7,                                    // 3 - corners 2, 7
                    VertexMask::V2 | VertexMask::V5,                                    // 4 - corners 2, 5
                    VertexMask::V0 | VertexMask::V1 | VertexMask::V3,                   // 5 - corners 0, 1, 3
                    VertexMask::V2 | VertexMask::V3 | VertexMask::V5,                   // 6 - corners 2, 3, 5
                    VertexMask::V3 | VertexMask::V5 | VertexMask::V6,                   // 7 - corners 3, 5, 6
                    VertexMask::V0 | VertexMask::V1 | VertexMask::V2 | VertexMask::V3,  // 8 - corners 0, 1, 2, 3
                    VertexMask::V0 | VertexMask::V1 | VertexMask::V2 | VertexMask::V4,  // 9 - corners 0, 1, 2, 4
                    VertexMask::V1 | VertexMask::V2 | VertexMask::V5 | VertexMask::V6,  // 10 - corners 1, 2, 5, 6
                    VertexMask::V0 | VertexMask::V1 | VertexMask::V2 | VertexMask::V5,  // 11 - corners 0, 1, 2, 5
                    VertexMask::V0 | VertexMask::V1 | VertexMask::V3 | VertexMask::V6,  // 12 - corners 0, 1, 3, 6
                    VertexMask::V1 | VertexMask::V2 | VertexMask::V4 | VertexMask::V7,  // 13 - corners 1, 2, 4, 7
                    VertexMask::V0 | VertexMask::V1 | VertexMask::V3 | VertexMask::V4,  // 14 - corners 0, 1, 3, 4
                }
            };
        }

        constexpr auto kCanonicalVertexMasks = makeCanonicalVertexMasks();

        consteval auto makeEdgeRotationsTable()
        {
            // 24 rotations x 12 edges
            std::array<std::array<Edge, kCubeEdgeCount>, 24> table{};

            for (std::size_t rotation = 0; rotation < table.size(); ++rotation) {
                for (std::size_t e = 0; e < kCubeEdgeCount; ++e) {
                    // original endpoints of edge e:
                    const auto& [v1, v2] = kCubeEdges[e];

                    // rotated endpoints:
                    const auto rotV1 = kVertexRotationsTable[rotation][toUnderlying(v1)];
                    const auto rotV2 = kVertexRotationsTable[rotation][toUnderlying(v2)];

                    // find which edge connects rv1 <-> rv2
                    for (std::size_t e2 = 0; e2 < kCubeEdgeCount; ++e2) {
                        const auto& [u1, u2] = kCubeEdges[e2];
                        if ((toUnderlying(u1) == toUnderlying(rotV1)
                            && toUnderlying(u2) == toUnderlying(rotV2))
                            ||
                            (toUnderlying(u1) == toUnderlying(rotV2)
                                && toUnderlying(u2) == toUnderlying(rotV1)))
                        {
                            table[rotation][e] = Edge(e2);
                            break;
                        }
                    }
                }
            }

            return table;
        }

        constexpr auto kEdgeRotationsTable = makeEdgeRotationsTable();

        template<VkFrontFace FrontFace>
        consteval auto makeBaseTriTable()
        {
            std::array<MC33Case, 15> table{};
            std::size_t tableIdx = 0;
            //  Case 0 - None
            {
                auto& c = table[tableIdx++];
                c.caseIdx = 0;
                std::size_t variantIdx = 0;
                {
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                }
                c.variantCount = variantIdx;
            }
            //  Case 1 - V2
            {
                auto& c = table[tableIdx++];
                c.caseIdx = 1;
                std::size_t variantIdx = 0;
                c.vertexMask = VertexMask::V2;
                {
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E23;
                }
                c.variantCount = variantIdx;
            }
            //  Case 2 - V2, V3
            {
                auto& c = table[tableIdx++];
                c.caseIdx = 2;
                std::size_t variantIdx = 0;
                c.vertexMask = VertexMask::V2 | VertexMask::V3;
                {
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E37;

                    edges[idx++] = Edge::E13;
                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E37;
                }
                c.variantCount = variantIdx;
            }
            //  Case 3 - V2, V7
            {
                auto& c = table[tableIdx++];
                c.caseIdx = 3;
                c.mask = AmbiguityMask::YPos;
                std::size_t variantIdx = 0;
                c.vertexMask = VertexMask::V2 | VertexMask::V7;
                //  Case 3.1
                {
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E23;

                    edges[idx++] = Edge::E57;
                    edges[idx++] = Edge::E37;
                    edges[idx++] = Edge::E67;
                }
                //  Case 3.2
                {
                    c.variantLUT[toUnderlying(AmbiguityMask::YPos)] = variantIdx;
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    //  "Far plane" faces

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E45;

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E15;

                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E57;

                    //  "Near plane" faces

                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E57;

                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E57;

                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E57;
                }
                c.variantCount = variantIdx;
            }
            //  Case 4 - V2, V5
            {
                auto& c = table[tableIdx++];
                c.caseIdx = 4;
                c.mask = AmbiguityMask::Interior;
				std::size_t variantIdx = 0;
                c.vertexMask = VertexMask::V2 | VertexMask::V5;
                //  Case 4.1.1
                {
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E23;

                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E57;
                }
                //  Case 4.1.2
                {
                    c.variantLUT[toUnderlying(AmbiguityMask::Interior)] = variantIdx;
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E67;

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E37;

                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E57;

                    edges[idx++] = Edge::E57;
                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E37;
                }
                c.variantCount = variantIdx;
            }
            //  Case 5 - V0, V1, V3
            {
                auto& c = table[tableIdx++];
                c.caseIdx = 5;
                std::size_t variantIdx = 0;
                c.vertexMask = VertexMask::V0 | VertexMask::V1 | VertexMask::V3;
                {
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E04;
                    edges[idx++] = Edge::E37;

                    edges[idx++] = Edge::E04;
                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E37;

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E37;
                }
                c.variantCount = variantIdx;
            }
            //  Case 6 - V2, V3, V5
            {
                auto& c = table[tableIdx++];
                c.caseIdx = 6;
                c.mask = AmbiguityMask::XPos | AmbiguityMask::Interior;
                std::size_t variantIdx = 0;
                c.vertexMask = VertexMask::V2 | VertexMask::V3 | VertexMask::V5;
                //  Case 6.1.1
                {
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E13;

                    edges[idx++] = Edge::E13;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E37;

                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E57;
                }
                //  Case 6.1.2
                {
                    c.variantLUT[toUnderlying(AmbiguityMask::Interior)] = variantIdx;
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    //  "Far plane" triangles

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E45;

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E15;

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E13;

					//  "Near plane" triangles

                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E57;

                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E37;
                    edges[idx++] = Edge::E57;

                    edges[idx++] = Edge::E37;
                    edges[idx++] = Edge::E13;
                    edges[idx++] = Edge::E57;

                    edges[idx++] = Edge::E13;
                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E57;
                }
                //  Case 6.2
                {
                    //  This variant is indepenedent of the interior test, thus we duplicate values to ensure proper lookup.
                    c.variantLUT[toUnderlying(AmbiguityMask::XPos)] = variantIdx;
                    c.variantLUT[toUnderlying(AmbiguityMask::XPos | AmbiguityMask::Interior)] = variantIdx;
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    //  "Far plane" triangles

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E45;

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E15;

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E13;

                    //  "Near plane" triangles

                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E57;

                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E37;
                    edges[idx++] = Edge::E57;
                }
                c.variantCount = variantIdx;
            }
            //  Case 7 - V3, V5, V6
            {
                auto& c = table[tableIdx++];
                c.caseIdx = 7;
                c.mask = AmbiguityMask::XPos | AmbiguityMask::YPos | AmbiguityMask::ZPos | AmbiguityMask::Interior;
                std::size_t variantIdx = 0;
                c.vertexMask = VertexMask::V3 | VertexMask::V5 | VertexMask::V6;
                //  Case 7.1
                {
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    edges[idx++] = Edge::E13;
                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E37;

                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E57;

                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E26;
                }
                //  Case 7.2
                {
                    //  This variant is indepenedent of the interior test, thus we duplicate values to ensure proper lookup.
                    c.variantLUT[toUnderlying(AmbiguityMask::ZPos)] = variantIdx;
                    c.variantLUT[toUnderlying(AmbiguityMask::ZPos | AmbiguityMask::Interior)] = variantIdx;
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    //  "Top" triangles

                    //  "Far plane" triangles

                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E45;

                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E15;

                    //  "Near plane" triangles

                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E15;

                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E57;

                    //  "Bottom" triangles

                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E26;
                }
                //  Case 7.3
                {
                    //  This variant is indepenedent of the interior test, thus we duplicate values to ensure proper lookup.
                    c.variantLUT[toUnderlying(AmbiguityMask::XPos | AmbiguityMask::ZPos)] = variantIdx;
                    c.variantLUT[toUnderlying(AmbiguityMask::XPos | AmbiguityMask::ZPos | AmbiguityMask::Interior)] = variantIdx;
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    //  "Far plane" triangles

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E46;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E45;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E15;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E13;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E13;
                    edges[idx++] = Edge::E23;

                    //  "Near plane" triangles

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E26;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E57;
                    edges[idx++] = Edge::E67;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E37;
                    edges[idx++] = Edge::E57;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E37;
                }
                //  Case 7.4.1
                {
                    //  This variant is indepenedent of the interior test, thus we duplicate values to ensure proper lookup.
                    c.variantLUT[toUnderlying(AmbiguityMask::XPos | AmbiguityMask::YPos | AmbiguityMask::ZPos)] = variantIdx;
                    c.variantLUT[toUnderlying(AmbiguityMask::XPos | AmbiguityMask::YPos | AmbiguityMask::ZPos | AmbiguityMask::Interior)] = variantIdx;
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    //  "Far plane" triangles

                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E23;

                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E57;

                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E23;

                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E13;

                    //  "Near plane" triangles

                    edges[idx++] = Edge::E57;
                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E37;
                }
                //  Case 7.4.2
                {
                    c.variantLUT[toUnderlying(AmbiguityMask::Interior)] = variantIdx;
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E67;

                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E45;

                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E57;

                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E37;

                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E37;
                    edges[idx++] = Edge::E23;

                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E57;
                    edges[idx++] = Edge::E37;

                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E37;
                    edges[idx++] = Edge::E13;

                    edges[idx++] = Edge::E37;
                    edges[idx++] = Edge::E57;
                    edges[idx++] = Edge::E15;

                    edges[idx++] = Edge::E37;
                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E13;

                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E57;
                }
                c.variantCount = variantIdx;
            }
            //  Case 8 - V0, V1, V2, V3
            {
                auto& c = table[tableIdx++];
                c.caseIdx = 8;
                std::size_t variantIdx = 0;
                c.vertexMask = VertexMask::V0 | VertexMask::V1 | VertexMask::V2 | VertexMask::V3;
                {
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E04;
                    edges[idx++] = Edge::E26;

                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E37;
                }
                c.variantCount = variantIdx;
            }
            //  Case 9 - V0, V1, V2, V4
            {
                auto& c = table[tableIdx++];
                c.caseIdx = 9;
                std::size_t variantIdx = 0;
                c.vertexMask = VertexMask::V0 | VertexMask::V1 | VertexMask::V2 | VertexMask::V4;
                {
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E26;

                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E23;

                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E15;

                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E13;
                }
                c.variantCount = variantIdx;
            }
            //  Case 10 - V1, V2, V5, V6
            {
                auto& c = table[tableIdx++];
                c.caseIdx = 10;
                c.mask = AmbiguityMask::ZNeg | AmbiguityMask::Interior;
                std::size_t variantIdx = 0;
                c.vertexMask = VertexMask::V1 | VertexMask::V2 | VertexMask::V5 | VertexMask::V6;
                //  Case 10.1.1
                {
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E01;
                    edges[idx++] = Edge::E57;

                    edges[idx++] = Edge::E57;
                    edges[idx++] = Edge::E01;
                    edges[idx++] = Edge::E13;

                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E02;

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E23;
                }
                //  Case 10.1.2
                {
                    c.variantLUT[toUnderlying(AmbiguityMask::Interior)] = variantIdx;
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    //  "Top" quad

                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E57;

                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E57;
                    edges[idx++] = Edge::E45;

                    //  "Far" quad

                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E01;
                    edges[idx++] = Edge::E02;

                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E01;

                    //  "Bottom" quad

                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E01;

                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E01;
                    edges[idx++] = Edge::E13;

                    //  "Near" quad

                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E57;
                    edges[idx++] = Edge::E67;

                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E13;
                    edges[idx++] = Edge::E57;
                }
                // Case 10.2
                {
                    //  This variant is indepenedent of the interior test, thus we duplicate values to ensure proper lookup.
                    c.variantLUT[toUnderlying(AmbiguityMask::ZNeg)] = variantIdx;
                    c.variantLUT[toUnderlying(AmbiguityMask::ZNeg | AmbiguityMask::Interior)] = variantIdx;
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    // "Far plane" triangles

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E46;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E67;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E01;
                    edges[idx++] = Edge::E02;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E01;

                    // "Near plane" triangles

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E23;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E13;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E13;
                    edges[idx++] = Edge::E57;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E57;
                    edges[idx++] = Edge::E45;
                }
                c.variantCount = variantIdx;
            }
            //  Case 11 - V0, V1, V2, V5
            {
                auto& c = table[tableIdx++];
                c.caseIdx = 11;
                std::size_t variantIdx = 0;
                c.vertexMask = VertexMask::V0 | VertexMask::V1 | VertexMask::V2 | VertexMask::V5;
                {
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E04;
                    edges[idx++] = Edge::E57;

                    edges[idx++] = Edge::E04;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E23;

                    edges[idx++] = Edge::E04;
                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E57;

                    edges[idx++] = Edge::E57;
                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E13;
                }
                c.variantCount = variantIdx;
            }
            //  Case 12 - V0, V1, V3, V6
            {
                auto& c = table[tableIdx++];
                c.caseIdx = 12;
                c.mask = AmbiguityMask::XNeg | AmbiguityMask::YPos | AmbiguityMask::Interior;
                std::size_t variantIdx = 0;
                c.vertexMask = VertexMask::V0 | VertexMask::V1 | VertexMask::V3 | VertexMask::V6;
                //  Case 12.1.1
                {
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E04;
                    edges[idx++] = Edge::E37;

                    edges[idx++] = Edge::E04;
                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E37;

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E37;

                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E26;
                }
                //  Case 12.1.2
                {
                    c.variantLUT[toUnderlying(AmbiguityMask::Interior)] = variantIdx;
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    //  "Far plane" triangles

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E46;

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E04;

                    edges[idx++] = Edge::E04;
                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E15;

                    //  "Near plane" triangles

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E23;

                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E23;

                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E37;

                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E37;
                    edges[idx++] = Edge::E15;

                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E46;
                }
                //  Case 12.2
                {
                    //  This variant is indepenedent of the interior test, thus we duplicate values to ensure proper lookup.
                    c.variantLUT[toUnderlying(AmbiguityMask::YPos)] = variantIdx;
                    c.variantLUT[toUnderlying(AmbiguityMask::YPos | AmbiguityMask::Interior)] = variantIdx;
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    //  Far left lip

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E04;
                    edges[idx++] = Edge::E26;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E23;

                    //  Near left lip

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E46;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E26;

                    //  Near right lip

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E67;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E37;

                    //  Far right lip

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E04;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E37;
                    edges[idx++] = Edge::E15;
                }
                //  Case 12.3
                {
                    //  This variant is indepenedent of the interior test, thus we duplicate values to ensure proper lookup.
                    c.variantLUT[toUnderlying(AmbiguityMask::XNeg)] = variantIdx;
                    c.variantLUT[toUnderlying(AmbiguityMask::XNeg | AmbiguityMask::Interior)] = variantIdx;
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    //  Raised far lip

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E04;
                    edges[idx++] = Edge::E46;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E67;

                    //  Raised near lip

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E26;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E02;

                    //  Flat near lip

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E23;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E37;

                    //  Flat far lip

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E37;
                    edges[idx++] = Edge::E15;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E04;
                }
                c.variantCount = variantIdx;
            }
            //  Case 13 - V1, V2, V4, V7
            {
                auto& c = table[tableIdx++];
                c.caseIdx = 13;
                c.mask = AmbiguityMask::All;
                std::size_t variantIdx = 0;
                c.vertexMask = VertexMask::V1 | VertexMask::V2 | VertexMask::V4 | VertexMask::V7;
                //  Case 13.1
                {
                    //  This variant is indepenedent of the interior test, thus we duplicate values to ensure proper lookup.
                    //  "Normal" entry exists here by default, as it's the empty mask with the value of 0, so we don't explicitly add it.
                    c.variantLUT[toUnderlying(AmbiguityMask::Interior)] = variantIdx;
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    edges[idx++] = Edge::E01;
                    edges[idx++] = Edge::E13;
                    edges[idx++] = Edge::E15;

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E23;

                    edges[idx++] = Edge::E04;
                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E46;

                    edges[idx++] = Edge::E57;
                    edges[idx++] = Edge::E37;
                    edges[idx++] = Edge::E67;
                }
                //  Case 13.2
                {
                    //  This variant is indepenedent of the interior test, thus we duplicate values to ensure proper lookup.
                    c.variantLUT[toUnderlying(AmbiguityMask::ZPos)] = variantIdx;
                    c.variantLUT[toUnderlying(AmbiguityMask::ZPos | AmbiguityMask::Interior)] = variantIdx;
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    //  Two separate triangles

                    edges[idx++] = Edge::E01;
                    edges[idx++] = Edge::E13;
                    edges[idx++] = Edge::E15;

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E23;

                    //  Top "near" quad

                    edges[idx++] = Edge::E37;
                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E04;

                    edges[idx++] = Edge::E37;
                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E46;

                    //  Top "far" quad

                    edges[idx++] = Edge::E37;
                    edges[idx++] = Edge::E04;
                    edges[idx++] = Edge::E45;

                    edges[idx++] = Edge::E37;
                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E57;
                }
                //  Case 13.3
                {
                    //  This variant is indepenedent of the interior test, thus we duplicate values to ensure proper lookup.
                    c.variantLUT[toUnderlying(AmbiguityMask::XNeg | AmbiguityMask::ZPos)] = variantIdx;
                    c.variantLUT[toUnderlying(AmbiguityMask::XNeg | AmbiguityMask::ZPos | AmbiguityMask::Interior)] = variantIdx;
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    //  One separate triangles

                    edges[idx++] = Edge::E01;
                    edges[idx++] = Edge::E13;
                    edges[idx++] = Edge::E15;

                    //  Bottom "far" quad

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E04;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E02;

                    //  Bottom "near" quad

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E26;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E23;

                    //  Top "near" quad

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E46;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E37;
                    edges[idx++] = Edge::E67;

                    //  Top "far" quad + triangle

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E57;
                    edges[idx++] = Edge::E37;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E57;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E04;
                    edges[idx++] = Edge::E45;
                }
                //  Case 13.4
                {
                    //  This variant is indepenedent of the interior test, thus we duplicate values to ensure proper lookup.
                    c.variantLUT[toUnderlying(AmbiguityMask::XNeg | AmbiguityMask::YNeg | AmbiguityMask::ZPos)] = variantIdx;
                    c.variantLUT[toUnderlying(AmbiguityMask::XNeg | AmbiguityMask::YNeg | AmbiguityMask::ZPos | AmbiguityMask::Interior)] = variantIdx;
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    //  Bottom left "far" quad

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E04;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E02;

                    //  Bottom left "near" quad

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E26;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E23;

                    //  Top "near" quad

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E46;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E37;
                    edges[idx++] = Edge::E67;

                    //  Top "far" quad

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E57;
                    edges[idx++] = Edge::E37;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E57;

                    //  Bottom right "far" quad

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E04;
                    edges[idx++] = Edge::E01;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E01;
                    edges[idx++] = Edge::E13;

                    //  Bottom right "near" quad

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E13;
                    edges[idx++] = Edge::E15;

                    edges[idx++] = Edge::ESp;
                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E45;
                }
                //  Case 13.5.1
                {
                    c.variantLUT[toUnderlying(AmbiguityMask::XNeg | AmbiguityMask::YPos | AmbiguityMask::ZNeg)] = variantIdx;
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    //  Bottom left single triangle

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E23;

                    //  Top right single triangle

                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E57;
                    edges[idx++] = Edge::E15;

                    //  Middle quad + triangles

                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E04;
                    edges[idx++] = Edge::E67;

                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E04;
                    edges[idx++] = Edge::E01;

                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E01;
                    edges[idx++] = Edge::E37;

                    edges[idx++] = Edge::E37;
                    edges[idx++] = Edge::E01;
                    edges[idx++] = Edge::E13;
                }
                //  Case 13.5.2
                {
                    c.variantLUT[toUnderlying(AmbiguityMask::XNeg | AmbiguityMask::YPos | AmbiguityMask::ZNeg | AmbiguityMask::Interior)] = variantIdx;
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    //  Top right single triangle

                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E57;
                    edges[idx++] = Edge::E15;

                    //  Bottom right tunnel

                    //  "Far plane" triangles

                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E04;
                    edges[idx++] = Edge::E26;

                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E04;
                    edges[idx++] = Edge::E02;

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E04;
                    edges[idx++] = Edge::E01;

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E01;
                    edges[idx++] = Edge::E23;

                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E01;
                    edges[idx++] = Edge::E13;

                    //  "Near plane" triangles

                    edges[idx++] = Edge::E46;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E67;

                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E26;
                    edges[idx++] = Edge::E23;

                    edges[idx++] = Edge::E67;
                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E37;

                    edges[idx++] = Edge::E37;
                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E13;
                }
                c.variantCount = variantIdx;
            }
            //  Case 14 - V0, V1, V3, V4
            {
                auto& c = table[tableIdx++];
                c.caseIdx = 14;
                std::size_t variantIdx = 0;
                c.vertexMask = VertexMask::V0 | VertexMask::V1 | VertexMask::V3 | VertexMask::V4;
                {
                    auto& edges = c.variants[variantIdx++];
                    edges.fill(Edge::End);
                    std::size_t idx = 0;

                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E45;
                    edges[idx++] = Edge::E46;

                    edges[idx++] = Edge::E02;
                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E46;

                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E46;

                    edges[idx++] = Edge::E23;
                    edges[idx++] = Edge::E15;
                    edges[idx++] = Edge::E37;
                }
                c.variantCount = variantIdx;
            }

            // Our Marching Cubes convention uses an axis convention which would require a transform
            // with a determinant of -1 to return to "conventional" Vulkan coordinate space, which means
            // that triangle winding need to be reversed, or the normals will end up facing inwards.
            if constexpr (FrontFace == VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE) {
                for (auto& c : table) {
                    for (u8 v = 0; v < c.variantCount; ++v) {
                        auto& edges = c.variants[v];
                        for (std::size_t i = 0; edges[i] != Edge::End; i += 3) {
                            std::swap(edges[i + 1], edges[i + 2]);
                        }
                    }
                }
            }

            return table;
        }

        static_assert(
            makeBaseTriTable<VK_FRONT_FACE_CLOCKWISE>()
            != makeBaseTriTable<VK_FRONT_FACE_COUNTER_CLOCKWISE>()
        );

        static_assert(
            makeBaseTriTable<VK_FRONT_FACE_CLOCKWISE>()
            != makeBaseTriTable<VK_FRONT_FACE_COUNTER_CLOCKWISE>()
        );

        constexpr auto kBaseTriTable = makeBaseTriTable<RenderConventions::kFrontFace>();

        consteval Edge rotatedEdge(Edge e, std::size_t rotIdx)
        {
            // each of the 12 edges gets sent to a new edge–ID
            if (e == Edge::ESp) {
                return Edge::ESp;
            }
            return kEdgeRotationsTable[rotIdx][toUnderlying(e)];
        }

        consteval AmbiguityMask rotatedFaceMask(
            AmbiguityMask m,
            std::size_t rotIdx
        )
        {
            AmbiguityMask out = AmbiguityMask::None;
            for (std::size_t f = 0; f < kCubeFaceCount; ++f) {
                if (bit_enum::hasBit(m, f)) {
                    Face wf = kFaceRotationsTable[rotIdx][f];
                    out = bit_enum::setBit(out, toUnderlying(wf));
                }
            }
            // keep the interior bit
            if (bit_enum::hasBit(m, kAmbiguityInteriorIdx)) {
                out = bit_enum::setBit(out, kAmbiguityInteriorIdx);
            }
            return out;
        }

        constexpr detail::AmbiguityMask unrotatedFaceMask(
            detail::AmbiguityMask m,
            std::size_t rotIdx
        )
        {
            using namespace detail;
            AmbiguityMask out = AmbiguityMask::None;

            // kFaceRotationsTable[rotIdx][c] = world face for canonical face c
            // So to invert: for each canonical face c, look at its world face w
            // and if bit w is set in m, set bit c in out.
            for (std::size_t c = 0; c < kCubeFaceCount; ++c) {
                Face w = kFaceRotationsTable[rotIdx][c];
                if (bit_enum::hasBit(m, toUnderlying(w))) {
                    out = bit_enum::setBit(out, c);
                }
            }
            // Preserve interior bit
            if (bit_enum::hasBit(m, kAmbiguityInteriorIdx)) {
                out = bit_enum::setBit(out, kAmbiguityInteriorIdx);
            }
            return out;
        }

        template<VkFrontFace FrontFace>
        consteval std::array<MC33Case, kCubeVertexStateCount> makeTriTable()
        {
            std::array<MC33Case, kCubeVertexStateCount> table{};

            for (std::size_t cube = 0; cube < table.size(); ++cube) {
                // 1) maybe complement to reduce to <= 4 bits set
                const bool complement = (std::popcount(cube) > 4);
                const u8 reduced = complement ? (~cube & std::numeric_limits<decltype(reduced)>::max()) : cube;
                const VertexMask reducedMask = VertexMask(reduced);

                // 2) find the base vertex-mask index and the rotation in which
                // one of the 16 'canonical' masks, when permuted, equals our 'reduced' mask.
                std::size_t foundMaskIdx = std::numeric_limits<std::size_t>::max();
                std::size_t foundRotIdx = std::numeric_limits<std::size_t>::max();
                bool found = false;

                for (std::size_t maskIdx = 0; maskIdx < kCanonicalVertexMasks.size(); ++maskIdx) {

                    const auto canonicalMask = kCanonicalVertexMasks[maskIdx];

                    for (std::size_t rotIdx = 0; rotIdx < kVertexRotationsTable.size(); ++rotIdx) {

                        if (rotatedVertexMask(canonicalMask, rotIdx) == reducedMask) {
                            foundMaskIdx = maskIdx;
                            foundRotIdx = rotIdx;
                            found = true;
                            break;
                        }
                    }

                    if (found) {
                        break;
                    }
                }

                // 3) build the rotated case for this cube
                const auto& base = kBaseTriTable[foundMaskIdx];
                MC33Case out{};
                out.caseIdx = base.caseIdx;
                out.rotIdx = foundRotIdx;
                out.complemented = complement;
                out.ambiguousFaces = base.ambiguousFaces;
                out.mask = rotatedFaceMask(base.mask, foundRotIdx);
				out.vertexMask = rotatedVertexMask(base.vertexMask, foundRotIdx);
				out.variants = base.variants;
                out.variantCount = base.variantCount;
                out.variantLUT = base.variantLUT;

                for (auto& face : out.ambiguousFaces) {
                    for (auto& v : face) {
                        v = rotatedVertex(v, foundRotIdx);
                    }
                }

                // rotate all variants
                for (std::size_t v = 0; v < base.variantCount; ++v) {
                    const auto& src = base.variants[v];
                    auto& dst = out.variants[v];

                    // copy-then-rotate; stop at Edge::End
                    for (std::size_t i = 0; ; i += 3) {
                        Edge e0 = src[i + 0];
                        if (e0 == Edge::End) {
                            dst[i] = Edge::End;
                            break;
                        }
                        Edge e1 = src[i + 1];
                        Edge e2 = src[i + 2];

                        // rotate edges (ESp is handled in rotatedEdge)
                        e0 = rotatedEdge(e0, foundRotIdx);
                        e1 = rotatedEdge(e1, foundRotIdx);
                        e2 = rotatedEdge(e2, foundRotIdx);

                        if (complement) {
                            // reverse triangle winding
                            dst[i + 0] = e0;
                            dst[i + 1] = e2;
                            dst[i + 2] = e1;
                        }
                        else {
                            dst[i + 0] = e0;
                            dst[i + 1] = e1;
                            dst[i + 2] = e2;
                        }
                    }
                }

                table[cube] = out;
            }
            return table;
        }

        constexpr auto kTriTable = makeTriTable<RenderConventions::kFrontFace>();

        constexpr f32 faceDecider(
            const std::array<f32, 4>& vals,
            f32 iso
        ) noexcept
        {
            const f32 f00 = vals[0] - iso;
            const f32 f10 = vals[1] - iso;
            const f32 f01 = vals[2] - iso;
            const f32 f11 = vals[3] - iso;

            return f00 * f11 - f10 * f01;
        }

        constexpr f32 interiorDecider(
            const std::array<f32, 8>& vals,
            f32 iso
        ) noexcept
        {
            // shift all values relative to iso
            std::array<f32, 8> f{};
            for (std::size_t i = 0; i < 8; ++i) {
                f[i] = vals[i] - iso;
            }

            // Vertex ordering V0..V7 must match your cube convention!
            const f32 d =
                f[0] * f[7] + f[3] * f[4] + f[1] * f[6] + f[2] * f[5]
                - f[4] * f[3] - f[5] * f[2] - f[6] * f[1] - f[7] * f[0];

            return d;
        }

        template<typename T>
        concept ScalarFieldCoord = std::convertible_to<T, f32>;

        template<typename T>
        concept ScalarField3D =
            std::invocable<T, f32, f32, f32>&&
            std::convertible_to<std::invoke_result_t<T, f32, f32, f32>, f32>;

        struct DenseEdgeCache final {
            std::size_t nx, ny, nz;
            std::size_t NX, NY, NZ;

            std::vector<u32> xEdges;
            std::vector<u32> yEdges;
            std::vector<u32> zEdges;

			static constexpr auto EMPTY = std::numeric_limits<u32>::max();

            DenseEdgeCache(
                std::size_t cx,
                std::size_t cy,
                std::size_t cz
            )
                : nx(cx), ny(cy), nz(cz),
                NX(cx + 1), NY(cy + 1), NZ(cz + 1)
            {
                xEdges.assign(nx * NY * NZ, EMPTY);
                yEdges.assign(NX * ny * NZ, EMPTY);
                zEdges.assign(NX * NY * nz, EMPTY);
            }

            auto& atX(std::size_t i, std::size_t j, std::size_t k)
            {
                return xEdges[(i * NY + j) * NZ + k];
            }
            auto& atY(std::size_t i, std::size_t j, std::size_t k)
            {
                return yEdges[(i * ny + j) * NZ + k];
            }
            auto& atZ(std::size_t i, std::size_t j, std::size_t k)
            {
                return zEdges[(i * NY + j) * nz + k];
            }
        };

        enum class EdgeOrientation : u8 {
            X,
            Y,
            Z,
        };

    } // namespace detail

    inline std::ostream& operator<<(std::ostream& os, detail::Vertex v) noexcept
    {
        using detail::Vertex;
        switch (v) {
        case Vertex::V0: os << "V0";  break;
        case Vertex::V1: os << "V1";  break;
        case Vertex::V2: os << "V2";  break;
        case Vertex::V3: os << "V3";  break;
        case Vertex::V4: os << "V4";  break;
        case Vertex::V5: os << "V5";  break;
        case Vertex::V6: os << "V6";  break;
        case Vertex::V7: os << "V7";  break;
        default: os << "Vertex(" << static_cast<int>(v) << ")"; break;
        }
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, detail::VertexMask mask) noexcept
    {
        os << '[';
        for(std::size_t bitIdx = 0; bitIdx < detail::kCubeVertexCount; ++bitIdx) {
            if (bit_enum::hasBit(mask, bitIdx)) {
                os << detail::Vertex(bitIdx) << ", ";
            }
		}
		os << ']';
        return os;
	}

    inline std::ostream& operator<<(std::ostream& os, detail::Edge e) noexcept
    {
        using detail::Edge;
        switch (e) {
        case Edge::E01: os << "E01";  break;
        case Edge::E13: os << "E13";  break;
        case Edge::E23: os << "E23";  break;
        case Edge::E02: os << "E02";  break;
        case Edge::E45: os << "E45";  break;
        case Edge::E57: os << "E57";  break;
        case Edge::E67: os << "E67";  break;
        case Edge::E46: os << "E46";  break;
        case Edge::E04: os << "E04";  break;
        case Edge::E15: os << "E15";  break;
        case Edge::E37: os << "E37";  break;
        case Edge::E26: os << "E26";  break;
        case Edge::End: os << "End";  break;
        default: os << "Edge(" << static_cast<int>(e) << ")"; break;
        }
        return os;
    }

    template<
        scene::Vertex VertexT,
        std::unsigned_integral IndexT
    >
    struct MeshData {
        std::vector<VertexT> vertices;
        std::vector<IndexT> indices;
    };

    template<
        scene::Vertex VertexT,
        std::unsigned_integral IndexT,
        detail::ScalarField3D SamplerT,
        detail::ScalarFieldCoord CoordT
    >
    inline MeshData<VertexT, IndexT> marchingCubes(
        SamplerT&& samplerFn,
		glm::vec<3, CoordT> gridSize,
        f32 iso,
        glm::vec3 origin,
        glm::vec3 spacing,
        std::stop_token st = {}
    )
    {
        using detail::kCubeVertexCount;
        using detail::kCubeEdgeCount;
		using detail::kCubeEdges;
        using detail::Axis;
        using detail::Vertex;
		using detail::Edge;
        using detail::kEdgeTable;
		using detail::kTriTable;

        std::vector<glm::vec3> outVerts{};
        std::vector<std::array<IndexT, 3>> outIdxs{};

        /// Corner offsets - unpack each vertex into its axis bit values
        constexpr std::array<
            glm::vec<3, std::underlying_type_t<Vertex>>,
            kCubeVertexCount
        > cornerOffset = []() {
            std::array<
                glm::vec<3, std::underlying_type_t<Vertex>>,
                kCubeVertexCount
            > offsets{};
            std::size_t idx{};
            for (const auto vertex : {
                    Vertex::V0, Vertex::V1, Vertex::V2, Vertex::V3,
                    Vertex::V4, Vertex::V5, Vertex::V6, Vertex::V7,
            }) {
                offsets[idx++] = {
                    bit_utils::bit(toUnderlying(vertex), toUnderlying(Axis::X)),
                    bit_utils::bit(toUnderlying(vertex), toUnderlying(Axis::Y)),
                    bit_utils::bit(toUnderlying(vertex), toUnderlying(Axis::Z)),
                };
            }
            return offsets;
        }();

        const auto chunkSize = gridSize;

		const glm::u64vec3 dims{
            static_cast<std::size_t>(gridSize.x) + 1ull,
            static_cast<std::size_t>(gridSize.y) + 1ull,
            static_cast<std::size_t>(gridSize.z) + 1ull,
        };

        std::vector<f32> field{};
        field.reserve(volume(dims));
        
        for (std::size_t z = 0; z < dims.z; ++z) {
            for (std::size_t y = 0; y < dims.y; ++y) {
                for (std::size_t x = 0; x < dims.x; ++x) {
                    glm::vec3 pw = origin + spacing * glm::vec3{ x, y, z };
                    field.emplace_back(std::invoke(samplerFn, pw.x, pw.y, pw.z));
                }
            }
        }

        static constexpr auto sample = [](
            const std::vector<f32>& v,
            const glm::u64vec3 dims,
            std::size_t x,
            std::size_t y,
            std::size_t z
        )
        {
            return v[
                linearize(
                    glm::vec<3, std::size_t>{ x, y, z },
                    dims
                )
            ];
        };

	    std::unordered_map<glm::i64vec3, u32> uniqueVerts{};
		uniqueVerts.reserve(volume(chunkSize));

        for (std::size_t i = 0; i < chunkSize.x - 1; ++i) {
            for (std::size_t j = 0; j < chunkSize.y - 1; ++j) {
                for (std::size_t k = 0; k < chunkSize.z - 1; ++k) {
                    if (st.stop_requested()) {
                        return {};
                    }
                    // compute cubeIndex
                    std::size_t cubeIndex = 0;
                    std::array<f32, kCubeVertexCount> vals{};
                    std::array<glm::vec3, kCubeVertexCount> pts{};

                    for (std::size_t v = 0; v < cornerOffset.size(); ++v) {
                        if (st.stop_requested()) {
                            return {};
                        }
                        const auto& off = cornerOffset[v];

                        const CoordT x = static_cast<CoordT>(i + off.x);
                        const CoordT y = static_cast<CoordT>(j + off.y);
                        const CoordT z = static_cast<CoordT>(k + off.z);

                        const f32 val = sample(field, dims, x, y, z);

                        const glm::vec3 pw = origin + spacing * glm::vec3{ x, y, z };

                        vals[v] = val;
                        pts[v] = pw;

                        if (val < iso) {
                            cubeIndex = bit_utils::setBit(cubeIndex, v);
                        }
                    }

                    // fetch which edges are intersected
                    auto edges = kEdgeTable[cubeIndex];

                    if (bit_enum::none(edges)) {
                        continue;
                    }

                    // interpolate vertices on intersected edges
					std::array<u32, kCubeEdgeCount + 1> vertList = {};
                    vertList.fill(std::numeric_limits<u32>::max());
                    for (std::size_t e = 0; e < kCubeEdges.size(); ++e) {
                        if (st.stop_requested()) {
                            return {};
                        }
                        if (!bit_enum::hasBit(edges, e)) {
                            continue;
                        }
                        const auto [v1, v2] = kCubeEdges[e];

                        // unique key per grid-edge
                        const f32 val1 = vals[toUnderlying(v1)];
                        const f32 val2 = vals[toUnderlying(v2)];
                        const glm::vec3& p1 = pts[toUnderlying(v1)];
                        const glm::vec3& p2 = pts[toUnderlying(v2)];
                        const f32 denom = val2 - val1;
                        const f32 t = (std::fabs(denom) > 1e-6f) ? (iso - val1) / denom : 0.5f;
                        const glm::vec3 p = glm::mix(p1, p2, t);

                        const f32 epsilon = 1e-4f * glm::compMin(spacing);
                        glm::i64vec3 key = glm::i64vec3(glm::round(p / epsilon));

                        auto [it, inserted] = uniqueVerts.try_emplace(key, static_cast<u32>(outVerts.size()));
                        if (inserted) {
                            outVerts.push_back(p);
                        }
                        vertList[e] = it->second;
                    }

                    const detail::MC33Case& c = detail::kTriTable[cubeIndex];
                    detail::AmbiguityMask mask = detail::AmbiguityMask::None;

                    auto testFace = [&](detail::Face which) {
                        // 3, 4, 6, 7, 10, 12, 13
                        /*if (c.caseIdx == 3
                            || c.caseIdx == 4
                            || c.caseIdx == 6
                            || c.caseIdx == 7
                            || c.caseIdx == 10
                            || c.caseIdx == 12
                            || c.caseIdx == 13)
                        {
                            std::println("Handling case {}...", c.caseIdx);
                        }*/
                        if (!bit_enum::hasBit(c.mask, toUnderlying(which))) {
                            return;
                        }
                        const auto& quad = c.ambiguousFaces[toUnderlying(which)];
                        std::array<f32, 4> fv = {
                            vals[toUnderlying(quad[0])],
                            vals[toUnderlying(quad[1])],
                            vals[toUnderlying(quad[2])],
                            vals[toUnderlying(quad[3])],
                        };
                        // Choose a consistent tie rule. Using >= 0 matches your earlier sign convention.
                        if (detail::faceDecider(fv, iso) >= 0.0f) {
                            mask = bit_enum::setBit(mask, toUnderlying(which));
                        }
                    };

                    // Run whichever face tests this case needs (mask gates them)
                    testFace(detail::Face::XPos);
                    testFace(detail::Face::XNeg);
                    testFace(detail::Face::YPos);
                    testFace(detail::Face::YNeg);
                    testFace(detail::Face::ZPos);
                    testFace(detail::Face::ZNeg);

                    // Interior (volume) test if required
                    if (bit_enum::hasBit(c.mask, detail::kAmbiguityInteriorIdx)) {
                        if (detail::interiorDecider(vals, iso) >= 0.0f) {
                            mask = bit_enum::setBit(mask, detail::kAmbiguityInteriorIdx);
                        }
                    }

                    auto canonicalMask = detail::unrotatedFaceMask(mask, c.rotIdx);

                    if (c.complemented && bit_enum::hasBit(c.mask, detail::kAmbiguityInteriorIdx)) {
                        canonicalMask = bit_enum::toggleBit(canonicalMask, detail::kAmbiguityInteriorIdx);
                    }

                    // Pick the variant
                    const auto variantIndex = c.variantLUT[toUnderlying(canonicalMask)];
                    const auto& variant = c.variants[variantIndex];

                    if (false || variantIndex != 0) {
                        /*std::println(
                            "Picking variant {} for cubeIndex {:#04x} (cubeMask {:#02b}, vertexMask {:#02b}, ambiguityMask {:#02b}) at ({}, {}, {})",
                            variantIndex, cubeIndex, toUnderlying(c.mask), toUnderlying(c.vertexMask), toUnderlying(mask), i, j, k
                        );*/
                    }
                    // Detect if this variant references ESp
                    bool needsSpecial = false;
                    for (std::size_t t = 0; ; t += 3) {
                        if (st.stop_requested()) {
                            return {};
                        }
                        const detail::Edge e = variant[t];
                        if (e == detail::Edge::End) {
                            break;
                        }
                        if (e == detail::Edge::ESp
                            || variant[t + 1] == detail::Edge::ESp
                            || variant[t + 2] == detail::Edge::ESp)
                        {
                            needsSpecial = true;
                            break;
                        }
                    }

                    // If needed, compute the special vertex once and store its index in vertList[kSpecialEdgeIdx]
                    if (needsSpecial) {
                        // Simple robust choice: cube center (works well for MC33’s “inside vertex”)
                        glm::vec3 pCenter = origin + spacing * glm::vec3{
                            static_cast<f32>(i) + 0.5f,
                            static_cast<f32>(j) + 0.5f,
                            static_cast<f32>(k) + 0.5f
                        };

                        const u32 idx = static_cast<u32>(outVerts.size());
                        outVerts.push_back(pCenter);
                        vertList[detail::kSpecialEdgeIdx] = idx;
                    }

                    for (std::size_t t = 0; ; t += 3) {
                        if (st.stop_requested()) {
                            return {};
                        }
                        detail::Edge e0 = variant[t];
                        if (e0 == detail::Edge::End) {
                            break;
                        }
                        detail::Edge e1 = variant[t + 1];
                        detail::Edge e2 = variant[t + 2];

                        constexpr auto m = std::numeric_limits<IndexT>::max();
                        const IndexT i0 = vertList[toUnderlying(e0)];
                        const IndexT i1 = vertList[toUnderlying(e1)];
                        const IndexT i2 = vertList[toUnderlying(e2)];
                        if (i0 == m || i1 == m || i2 == m) {
                            // Shouldn't happen unless the table references an edge that wasn't intersected
                            // (ESp is handled by the pre-materialization above)
                            continue;
                        }
                        outIdxs.push_back({ { i0, i1, i2 } });
                    }
                }
            }
        }

        MeshData<VertexT, IndexT> result{};
        result.vertices.reserve(outVerts.size());
        result.indices.reserve(outIdxs.size() * 3);

        auto gradientAt = [&](const glm::vec3& p) -> glm::vec3 {
            // step relative to grid spacing
            const auto h = spacing * 0.5f;
            const f32 fx = std::invoke(samplerFn, p.x + h.x, p.y, p.z) - std::invoke(samplerFn, p.x - h.x, p.y, p.z);
            const f32 fy = std::invoke(samplerFn, p.x, p.y + h.y, p.z) - std::invoke(samplerFn, p.x, p.y - h.y, p.z);
            const f32 fz = std::invoke(samplerFn, p.x, p.y, p.z + h.z) - std::invoke(samplerFn, p.x, p.y, p.z - h.z);
            glm::vec3 g = glm::vec3{ fx / (2 * h.x), fy / (2 * h.y), fz / (2 * h.z) };
            // For iso-surfaces defined as f<iso, outward normal is ±normalize(grad).
            // If your triangles are CCW and you want outward normals, use -grad when f decreases inward.
            return glm::length2(g) > 0 ? glm::normalize(g) : glm::vec3{ 0, 1, 0 };
        };

        for (const auto& v : outVerts) {
            if (st.stop_requested()) {
                return {};
            }
            // You can populate normals/texcoords later if needed
            auto normal = gradientAt(v);
            VertexT vert{};
            vert.get<scene::Position>() = v;
            vert.get<scene::Normal>() = normal;
            result.vertices.push_back(vert);
        }

        for (const auto& tri : outIdxs) {
            if (st.stop_requested()) {
                return {};
            }
            result.indices.push_back(tri[0]);
            result.indices.push_back(tri[1]);
            result.indices.push_back(tri[2]);
        }

        return result;
    }
}

#endif // !MARCHING_CUBES_UTILS_MARCHING_CUBES_HPP
