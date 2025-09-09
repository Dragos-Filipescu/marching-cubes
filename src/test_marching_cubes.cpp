//// test_marching_cubes.cpp
//
//#include <algorithm>
//#include <array>
//#include <bit>
//#include <bitset>
//#include <cstddef>
//#include <cstdint>
//#include <iostream>
//#include <set>
//#include <utility>
//#include <vector>
//
//#include <utils/bit_enum.hpp>
//#include <utils/bit_utils.hpp>
//#include <utils/enum_utils.hpp>
//
//#include <utils/marching_cubes.hpp>
//
//using namespace ::marching_cubes;
//using namespace ::marching_cubes::utils;
////
////// (Bring in whatever headers define your enums, bit_enum::hasBit, enum_utils::toUnderlying…)
////// For this standalone test, I’ll re-declare only the essentials:
////
////enum class Vertex : std::uint8_t {
////    V0 = 0,
////    V1 = 1,
////    V2 = 2,
////    V3 = 3,
////    V4 = 4,
////    V5 = 5,
////    V6 = 6,
////    V7 = 7,
////};
////
////enum class VertexMask : std::uint8_t {
////    None = 0,
////    V0 = 1 << enum_utils::toUnderlying(Vertex::V0),
////    V1 = 1 << enum_utils::toUnderlying(Vertex::V1),
////    V2 = 1 << enum_utils::toUnderlying(Vertex::V2),
////    V3 = 1 << enum_utils::toUnderlying(Vertex::V3),
////    V4 = 1 << enum_utils::toUnderlying(Vertex::V4),
////    V5 = 1 << enum_utils::toUnderlying(Vertex::V5),
////    V6 = 1 << enum_utils::toUnderlying(Vertex::V6),
////    V7 = 1 << enum_utils::toUnderlying(Vertex::V7),
////    All = (V0 | V1 | V2 | V3 | V4 | V5 | V6 | V7),
////    BIT_ENUM_TAG,
////};
////
////enum class Edge : std::uint8_t {
////    E01 = 0,    // Edge between corners 0 – 1
////    E13 = 1,    // Edge between corners 1 – 2
////    E23 = 2,    // Edge between corners 2 – 3
////    E02 = 3,    // Edge between corners 3 – 0
////    E45 = 4,    // Edge between corners 4 – 5
////    E57 = 5,    // Edge between corners 5 – 6
////    E67 = 6,    // Edge between corners 6 – 7
////    E46 = 7,    // Edge between corners 7 – 4
////    E04 = 8,    // Edge between corners 0 – 4
////    E15 = 9,    // Edge between corners 1 – 5
////    E26 = 10,   // Edge between corners 2 – 6
////    E37 = 11,   // Edge between corners 3 – 7
////    End = 255,  // Stop condition, no more triangles
////};
////
////enum class EdgeMask : std::uint16_t {
////    None = 0,
////    E01 = 1u << enum_utils::toUnderlying(Edge::E01),   // Edge between corners 0 – 1
////    E13 = 1u << enum_utils::toUnderlying(Edge::E13),   // Edge between corners 1 – 2
////    E23 = 1u << enum_utils::toUnderlying(Edge::E23),   // Edge between corners 2 – 3
////    E02 = 1u << enum_utils::toUnderlying(Edge::E02),   // Edge between corners 3 – 0
////    E45 = 1u << enum_utils::toUnderlying(Edge::E45),   // Edge between corners 4 – 5
////    E57 = 1u << enum_utils::toUnderlying(Edge::E57),   // Edge between corners 5 – 6
////    E67 = 1u << enum_utils::toUnderlying(Edge::E67),   // Edge between corners 6 – 7
////    E46 = 1u << enum_utils::toUnderlying(Edge::E46),   // Edge between corners 7 – 4
////    E04 = 1u << enum_utils::toUnderlying(Edge::E04),   // Edge between corners 0 – 4
////    E15 = 1u << enum_utils::toUnderlying(Edge::E15),   // Edge between corners 1 – 5
////    E26 = 1u << enum_utils::toUnderlying(Edge::E26),   // Edge between corners 2 – 6
////    E37 = 1u << enum_utils::toUnderlying(Edge::E37),   // Edge between corners 3 – 7
////    All = (E01 | E13 | E23 | E02 | E45 | E57 | E67 | E46 | E04 | E15 | E26 | E37),
////    BIT_ENUM_TAG,
////};
////
////INCLUDE_BIT_ENUM_BIT_OPS;
////INCLUDE_BIT_ENUM_COMPARISON;
////
////enum class Axis : std::uint8_t {
////    X = 0,
////    Y = 1,
////    Z = 2,
////};
////
////enum class AxisFlip : std::int8_t {
////    None = +1,  // no flip
////    Flip = -1,  // flip the axis
////};
////
////constexpr std::array<
////    std::pair<
////    Vertex,
////    Vertex
////    >,
////    12
////> cubeEdges{
////    {
////        /// Bottom face
////        { Vertex::V0, Vertex::V1 }, // E01 (00)
////        { Vertex::V1, Vertex::V3 }, // E13 (01)
////        { Vertex::V2, Vertex::V3 }, // E23 (02)
////        { Vertex::V0, Vertex::V2 }, // E02 (03)
////
////        /// Top face
////        { Vertex::V4, Vertex::V5 }, // E45 (04)
////        { Vertex::V5, Vertex::V7 }, // E57 (05)
////        { Vertex::V6, Vertex::V7 }, // E67 (06)
////        { Vertex::V4, Vertex::V6 }, // E46 (07)
////
////        /// Connecting edges
////        { Vertex::V0, Vertex::V4 }, // E04 (08)
////        { Vertex::V1, Vertex::V5 }, // E15 (09)
////        { Vertex::V2, Vertex::V6 }, // E26 (10)
////        { Vertex::V3, Vertex::V7 }, // E37 (11)
////    }
////};
////
////static auto makeAxisPermutations()
////{
////    // start in lexicographical order:
////    std::array<Axis, 3> axes{
////        {
////            Axis::X,
////            Axis::Y,
////            Axis::Z,
////        }
////    };
////    // we'll collect 6 permutations:
////    std::array<std::array<Axis, 3>, 6> permutations{};
////    std::size_t i = 0;
////
////    do {
////        permutations[i++] = axes;
////    } while (
////        std::next_permutation(
////            axes.begin(), axes.end(),
////            [](const Axis& lhs, const Axis& rhs) {
////                return enum_utils::toUnderlying(lhs)
////                    < enum_utils::toUnderlying(rhs);
////            }
////        )
////    );
////    return permutations;
////}
////
////static auto permutationParity(const std::array<Axis, 3>& permutation) noexcept
////{
////    enum class PermutationParity : std::int8_t {
////        Even = +1,
////        Odd = -1,
////    };
////
////    std::int8_t invertedAxes = 0;
////    for (std::size_t i = 0; i < permutation.size(); ++i) {
////        for (std::size_t j = i + 1; j < permutation.size(); ++j) {
////            if (permutation[j] < permutation[i]) {
////                ++invertedAxes;
////            }
////        }
////    }
////    return ((invertedAxes & 1) == 0 ? PermutationParity::Even : PermutationParity::Odd);
////}
////
////static Vertex indexOf(bool x, bool y, bool z)
////{
////    return Vertex(
////        (static_cast<std::size_t>(x) << enum_utils::toUnderlying(Axis::X))
////        | (static_cast<std::size_t>(y) << enum_utils::toUnderlying(Axis::Y))
////        | (static_cast<std::size_t>(z) << enum_utils::toUnderlying(Axis::Z))
////    );
////}
////
////static auto makeVertexPermutationsTable()
////{
////    /// Each of the 6 permutations describes a rotation of the cube's axes, as such:
////    /// {X, Y, Z} -> {X, Z, Y} means "rotate the cube until the old Y axis is now
////    /// aligned with the Z axis, and the old Z axis is now aligned with the Y axis"
////    auto axisPermutations = makeAxisPermutations();
////
////    constexpr std::array<
////        std::array<AxisFlip, 3>,
////        8
////    > flipPermutations{
////        {
////            //  X axis flip ,   Y axis flip,    Z axis flip
////            {{  AxisFlip::None, AxisFlip::None, AxisFlip::None  }},
////            {{  AxisFlip::Flip, AxisFlip::None, AxisFlip::None  }},
////            {{  AxisFlip::None, AxisFlip::Flip, AxisFlip::None  }},
////            {{  AxisFlip::None, AxisFlip::None, AxisFlip::Flip  }},
////            {{  AxisFlip::Flip, AxisFlip::Flip, AxisFlip::None  }},
////            {{  AxisFlip::Flip, AxisFlip::None, AxisFlip::Flip  }},
////            {{  AxisFlip::None, AxisFlip::Flip, AxisFlip::Flip  }},
////            {{  AxisFlip::Flip, AxisFlip::Flip, AxisFlip::Flip  }},
////        }
////    };
////
////    std::array<
////        std::array<Vertex, 8>,
////        24
////    > table{};
////    std::size_t idx = 0;
////
////    for (
////        const auto [
////            xAxisPermutation,
////            yAxisPermutation,
////            zAxisPermutation
////        ] : axisPermutations
////        ) {
////        auto parity = permutationParity(
////            std::array<Axis, 3>{
////                {
////                    xAxisPermutation,
////                        yAxisPermutation,
////                        zAxisPermutation,
////                }
////        }
////        );
////        for (
////            const auto [
////                xAxisFlip,
////                yAxisFlip,
////                zAxisFlip
////            ] : flipPermutations
////            ) {
////            auto determinant = enum_utils::toUnderlying(parity)
////                * enum_utils::toUnderlying(xAxisFlip)
////                * enum_utils::toUnderlying(yAxisFlip)
////                * enum_utils::toUnderlying(zAxisFlip);
////
////            if (determinant != 1) {
////                // skip this permutation, it’s not a valid rotation
////                continue;
////            }
////            // for each of the 8 source vertices...
////            for (std::uint8_t v = 0; v < 8; ++v) {
////                // unpack into centered coords [-1, +1]
////                const std::array<const std::int8_t, 3> coords = {
////                    {
////                        bit_utils::hasBit(v, enum_utils::toUnderlying(Axis::X)) ? +1 : -1,
////                        bit_utils::hasBit(v, enum_utils::toUnderlying(Axis::Y)) ? +1 : -1,
////                        bit_utils::hasBit(v, enum_utils::toUnderlying(Axis::Z)) ? +1 : -1,
////                    }
////                };
////                // apply axis‐perm + sign
////                auto newX = enum_utils::toUnderlying(xAxisFlip)
////                    * coords[enum_utils::toUnderlying(xAxisPermutation)];
////                auto newY = enum_utils::toUnderlying(yAxisFlip)
////                    * coords[enum_utils::toUnderlying(yAxisPermutation)];
////                auto newZ = enum_utils::toUnderlying(zAxisFlip)
////                    * coords[enum_utils::toUnderlying(zAxisPermutation)];
////                // map back from [-1, +1] -> [0, 1]
////                bool xFinal = newX > 0;
////                bool yFinal = newY > 0;
////                bool zFinal = newZ > 0;
////                table[idx][v] = indexOf(xFinal, yFinal, zFinal);
////            }
////            idx++;
////        }
////    }
////    return table;
////}
////
////auto vertexPermutationsTable = makeVertexPermutationsTable();
////
////static auto makeEdgePermutationsTable()
////{
////    // 24 rotations x 12 edges
////    constexpr auto edgeCount = cubeEdges.size();
////    std::array<std::array<Edge, edgeCount>, 24> table{};
////
////    for (std::size_t rotation = 0; rotation < table.size(); ++rotation) {
////        for (std::size_t e = 0; e < edgeCount; ++e) {
////            // original endpoints of edge e:
////            const auto& [v1, v2] = cubeEdges[e];
////
////            // rotated endpoints:
////            const auto rotV1 = vertexPermutationsTable[rotation][enum_utils::toUnderlying(v1)];
////            const auto rotV2 = vertexPermutationsTable[rotation][enum_utils::toUnderlying(v2)];
////
////            // find which edge connects rv1 <-> rv2
////            for (std::size_t e2 = 0; e2 < edgeCount; ++e2) {
////                const auto& [u1, u2] = cubeEdges[e2];
////                if ((enum_utils::toUnderlying(u1) == enum_utils::toUnderlying(rotV1)
////                    && enum_utils::toUnderlying(u2) == enum_utils::toUnderlying(rotV2))
////                    ||
////                    (enum_utils::toUnderlying(u1) == enum_utils::toUnderlying(rotV2)
////                        && enum_utils::toUnderlying(u2) == enum_utils::toUnderlying(rotV1)))
////                {
////                    table[rotation][e] = Edge(e2);
////                    break;
////                }
////            }
////        }
////    }
////
////    return table;
////}
////
////auto edgePermutationsTable = makeEdgePermutationsTable();
////
////static auto makeInverseRotations()
////{
////    constexpr auto N = vertexPermutationsTable.size();
////    std::array<std::size_t, N> inv{};
////
////    for (std::size_t rotIdx = 0; rotIdx < N; ++rotIdx) {
////        // look for s such that rotating by r then by s yields identity
////        for (std::size_t inverseIdx = 0; inverseIdx < N; ++inverseIdx) {
////            bool matches = true;
////            // test on each of the 8 vertices
////            for (std::size_t v = 0; v < 8; ++v) {
////                // apply s then r
////                const auto mid = vertexPermutationsTable[inverseIdx][v];
////                const auto fin = vertexPermutationsTable[rotIdx][enum_utils::toUnderlying(mid)];
////                if (enum_utils::toUnderlying(fin) != v) {
////                    matches = false;
////                    break;
////                }
////            }
////            if (matches) {
////                inv[rotIdx] = inverseIdx;
////                break;
////            }
////        }
////    }
////
////    return inv;
////}
////
////static VertexMask rotatedVertex(VertexMask mask, std::size_t rotIdx)
////{
////    // pull out each bit, send it to its permuted slot
////    VertexMask out = VertexMask::None;
////    constexpr auto vertexCount = std::tuple_size_v<decltype(vertexPermutationsTable)::value_type>;
////    for (std::size_t v = 0; v < vertexCount; ++v) {
////        if (bit_enum::hasBit(mask, v)) {
////            out = bit_enum::setBit(out, enum_utils::toUnderlying(vertexPermutationsTable[rotIdx][v]));
////        }
////    }
////    return out;
////}
////
////static std::array<VertexMask, 16> canonicalVertexMasks = { {
////    VertexMask{0x00},              // 00000000 — empty
////    VertexMask{0x01},              // 00000001 — {V0}
////    VertexMask{0x03},              // 00000011 — {V0,V1}
////    VertexMask{0x05},              // 00000101 — {V0,V2}
////    VertexMask{0x06},              // 00000110 — {V1,V2}
////    VertexMask{0x07},              // 00000111 — {V0,V1,V2} (face triangle)
////    VertexMask{0x09},              // 00001001 — {V0,V3}
////    VertexMask{0x0A},              // 00001010 — {V1,V3}
////    VertexMask{0x0C},              // 00001100 — {V2,V3}
////    VertexMask{0x0F},              // 00001111 — {V0,V1,V2,V3} (full face)  ← **NEW**
////    VertexMask{0x11},              // 00010001 — {V0,V4}
////    VertexMask{0x12},              // 00010010 — {V1,V4}
////    VertexMask{0x14},              // 00010100 — {V2,V4}
////    VertexMask{0x16},              // 00010110 — {V1,V2,V4} (equilateral triangle)
////    VertexMask{0x18},              // 00011000 — {V3,V4}
////    VertexMask{0x19},              // 00011001 — {V0,V3,V4}
////} };
////
////// now produce the constexpr inverse‐rotation table
//////static auto inverseRotation = makeInverseRotations();
////
////static constexpr std::array<std::array<Edge, 16>, 16> baseTriTable{
////    {
////        // case 0: 00000000 — no corners inside
////        {
////            {
////                Edge::End, Edge::End, Edge::End, Edge::End,
////                Edge::End, Edge::End, Edge::End, Edge::End,
////                Edge::End, Edge::End, Edge::End, Edge::End,
////                Edge::End, Edge::End, Edge::End, Edge::End,
////            }
////        },
////
////    // case 1: 00000001 — corner 0
////    {
////        {
////            Edge::E01, Edge::E04, Edge::E02, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////        }
////    },
////
////    // case 2: 00000010 — corner 1
////    {
////        {
////            Edge::E01, Edge::E13, Edge::E15, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////        }
////    },
////
////    // case 3: 00000011 — corners 0 & 1
////    {
////        {
////            Edge::E13, Edge::E04, Edge::E02, Edge::E15,
////            Edge::E04, Edge::E13, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////        }
////    },
////
////    // case 4: 00000100 — corner 2
////    {
////        {
////            Edge::E13, Edge::E23, Edge::E26, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////        }
////    },
////
////    // case 5: 00000101 — corners 0 & 2
////    {
////        {
////            Edge::E01, Edge::E04, Edge::E02, Edge::E13,
////            Edge::E23, Edge::E26, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////        }
////    },
////
////    // case 6: 00000110 — corners 1 & 2
////    {
////        {
////            Edge::E15, Edge::E23, Edge::E26, Edge::E01,
////            Edge::E23, Edge::E15, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////        }
////    },
////
////    // case 7: 00000111 — corners 0,1,2
////    {
////        {
////            Edge::E23, Edge::E04, Edge::E02, Edge::E23,
////            Edge::E26, Edge::E04, Edge::E26, Edge::E15,
////            Edge::E04, Edge::End, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////        }
////    },
////
////    // case 8: 00001000 — corner 3
////    {
////        {
////            Edge::E02, Edge::E37, Edge::E23, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////        }
////    },
////
////    // case 9: 00001001 — corners 0 & 3
////    {
////        {
////            Edge::E01, Edge::E37, Edge::E23, Edge::E04,
////            Edge::E37, Edge::E01, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////        }
////    },
////
////    // case 10: 00001010 — corners 1 & 3
////    {
////        {
////            Edge::E13, Edge::E15, Edge::E01, Edge::E23,
////            Edge::E02, Edge::E37, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////        }
////    },
////
////    // case 11: 00001011 — corners 0,1,3
////    {
////        {
////            Edge::E13, Edge::E37, Edge::E23, Edge::E13,
////            Edge::E15, Edge::E37, Edge::E15, Edge::E04,
////            Edge::E37, Edge::End, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////        }
////    },
////
////    // case 12: 00001100 — corners 2 & 3
////    {
////        {
////            Edge::E02, Edge::E26, Edge::E13, Edge::E37,
////            Edge::E26, Edge::E02, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////        }
////    },
////
////    // case 13: 00001101 — corners 0,2,3
////    {
////        {
////            Edge::E01, Edge::E26, Edge::E13, Edge::E01,
////            Edge::E04, Edge::E26, Edge::E04, Edge::E37,
////            Edge::E26, Edge::End, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////        }
////    },
////
////    // case 14: 00001110 — corners 1,2,3
////    {
////        {
////            Edge::E02, Edge::E15, Edge::E01, Edge::E02,
////            Edge::E37, Edge::E15, Edge::E37, Edge::E26,
////            Edge::E15, Edge::End, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////        }
////    },
////
////    // case 15: 00001111 — corners 0,1,2,3
////    {
////        {
////            Edge::E15, Edge::E04, Edge::E26, Edge::E26,
////            Edge::E04, Edge::E37, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////            Edge::End, Edge::End, Edge::End, Edge::End,
////        }
////    },
////}
////};
////
////static std::uint8_t reverseWinding(std::uint8_t idx)
////{
////    // Given an index idx into triTable[cube][slot], where slot runs 0..15:
////    //   - slots 0,1,2 are the first triangle (edges [0],[1],[2])
////    //   - slots 3,4,5 are the second triangle, etc.
////    // We only ever call this on slots 0..2 (i%3), so:
////    constexpr std::array<std::uint8_t, 3> remap = { { 0, 2, 1 } };
////    return remap[idx % 3u] + 3u * (idx / 3u);
////}
////
////static Edge rotatedEdgeId(std::size_t edgeIdx, std::size_t rotIdx)
////{
////    // each of the 12 edges gets sent to a new edge–ID
////    return edgePermutationsTable[rotIdx][edgeIdx];
////}
////
////static auto makeTriTable()
////{
////    std::array<std::array<Edge, 16>, 256> table{};
////
////    static auto inverseRotation = makeInverseRotations();
////
////    for (std::size_t cube = 0; cube < table.size(); ++cube) {
////        // 1) maybe complement to reduce to <= 4 bits set
////        const bool complement = (std::popcount(cube) > 4);
////        const std::uint8_t reduced = complement ? (~cube & std::numeric_limits<decltype(reduced)>::max()) : cube;
////        const VertexMask reducedMask = VertexMask{ reduced };
////
////        // 2) find the base vertex-mask index and the rotation in which our `reduced` mask,
////        // when permuted, equals one of the 16 canonical `vertexMasks`.
////        std::size_t foundMaskIdx = std::numeric_limits<std::size_t>::max();
////        std::size_t foundRotIdx = 0;
////        bool found = false;
////
////        // 2a) first see if it matches a canonical *without* rotation:
////        for (std::size_t maskIdx = 0; maskIdx < canonicalVertexMasks.size(); ++maskIdx) {
////            if (canonicalVertexMasks[maskIdx] == reducedMask) {
////                foundMaskIdx = maskIdx;
////                foundRotIdx = 0;
////                break;
////            }
////        }
////
////        // 2b) otherwise, fall back to the full rotate-and-match:
////        if (foundMaskIdx == std::numeric_limits<std::size_t>::max()) {
////            for (std::size_t maskIdx = 0; maskIdx < canonicalVertexMasks.size(); ++maskIdx) {
////				const auto canonicalMask = canonicalVertexMasks[maskIdx];
////                for (std::size_t rotIdx = 0; rotIdx < vertexPermutationsTable.size(); ++rotIdx) {
////                    const auto inv = inverseRotation[rotIdx];
////                    if (rotatedVertex(reducedMask, inv) == canonicalMask) {
////                        foundMaskIdx = maskIdx;
////                        foundRotIdx = rotIdx;
////                        break;
////                    }
////                }
////                if (foundMaskIdx != std::numeric_limits<std::size_t>::max()) {
////                    break;
////                }
////            }
////        }
////
////        // 3) now pull the 16 edge‐indices from baseTriTable[foundMaskIdx],
////        //    apply the same complement (which just flips the winding
////        //    order) and rotation to *each* edge index, and store into
////        //    table[cube].
////        for (std::size_t slot = 0; slot < baseTriTable[foundMaskIdx].size(); ++slot) {
////            auto e = baseTriTable[foundMaskIdx][slot];
////            if (e == Edge::End) {
////                table[cube][slot] = Edge::End;
////            }
////            else {
////                // get the raw integer edge‐ID
////                auto raw = enum_utils::toUnderlying(e);
////                // maybe flip winding
////                auto fixed = complement
////                    ? reverseWinding(raw)
////                    : raw;
////                // then rotate the edge‐ID itself
////                table[cube][slot] = rotatedEdgeId(fixed, foundRotIdx);
////            }
////        }
////    }
////    return table;
////}
////
////static constexpr std::array<
////    std::array<
////        Edge,
////        16
////    >,
////    256
////> referenceTriTable = { {
////    {{Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E04, Edge::E02, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E13, Edge::E15, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E04, Edge::E02, Edge::E15, Edge::E04, Edge::E13, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E23, Edge::E26, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E04, Edge::E02, Edge::E13, Edge::E23, Edge::E26, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E23, Edge::E26, Edge::E01, Edge::E23, Edge::E15, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E23, Edge::E04, Edge::E02, Edge::E23, Edge::E26, Edge::E04, Edge::E26, Edge::E15, Edge::E04, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E02, Edge::E37, Edge::E23, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E37, Edge::E23, Edge::E04, Edge::E37, Edge::E01, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E15, Edge::E01, Edge::E23, Edge::E02, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E37, Edge::E23, Edge::E13, Edge::E15, Edge::E37, Edge::E15, Edge::E04, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E02, Edge::E26, Edge::E13, Edge::E37, Edge::E26, Edge::E02, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E26, Edge::E13, Edge::E01, Edge::E04, Edge::E26, Edge::E04, Edge::E37, Edge::E26, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E02, Edge::E15, Edge::E01, Edge::E02, Edge::E37, Edge::E15, Edge::E37, Edge::E26, Edge::E15, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E04, Edge::E26, Edge::E26, Edge::E04, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E45, Edge::E46, Edge::E04, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E45, Edge::E02, Edge::E01, Edge::E46, Edge::E02, Edge::E45, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E13, Edge::E15, Edge::E04, Edge::E45, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E45, Edge::E13, Edge::E15, Edge::E45, Edge::E46, Edge::E13, Edge::E46, Edge::E02, Edge::E13, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E23, Edge::E26, Edge::E04, Edge::E45, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E02, Edge::E45, Edge::E46, Edge::E02, Edge::E01, Edge::E45, Edge::E13, Edge::E23, Edge::E26, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E23, Edge::E26, Edge::E15, Edge::E01, Edge::E23, Edge::E04, Edge::E45, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E23, Edge::E26, Edge::E15, Edge::E23, Edge::E15, Edge::E46, Edge::E23, Edge::E46, Edge::E02, Edge::E46, Edge::E15, Edge::E45, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E04, Edge::E45, Edge::E46, Edge::E02, Edge::E37, Edge::E23, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E37, Edge::E45, Edge::E46, Edge::E37, Edge::E23, Edge::E45, Edge::E23, Edge::E01, Edge::E45, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E01, Edge::E13, Edge::E04, Edge::E45, Edge::E46, Edge::E23, Edge::E02, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E45, Edge::E46, Edge::E37, Edge::E15, Edge::E45, Edge::E37, Edge::E15, Edge::E37, Edge::E23, Edge::E15, Edge::E23, Edge::E13, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E02, Edge::E26, Edge::E13, Edge::E02, Edge::E37, Edge::E26, Edge::E46, Edge::E04, Edge::E45, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E37, Edge::E26, Edge::E13, Edge::E45, Edge::E37, Edge::E13, Edge::E01, Edge::E45, Edge::E46, Edge::E37, Edge::E45, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E45, Edge::E46, Edge::E04, Edge::E15, Edge::E01, Edge::E37, Edge::E15, Edge::E37, Edge::E26, Edge::E37, Edge::E01, Edge::E02, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E45, Edge::E46, Edge::E37, Edge::E45, Edge::E37, Edge::E15, Edge::E15, Edge::E37, Edge::E26, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E57, Edge::E45, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E57, Edge::E45, Edge::E01, Edge::E04, Edge::E02, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E57, Edge::E45, Edge::E13, Edge::E57, Edge::E01, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E04, Edge::E57, Edge::E45, Edge::E04, Edge::E02, Edge::E57, Edge::E02, Edge::E13, Edge::E57, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E23, Edge::E26, Edge::E15, Edge::E57, Edge::E45, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E02, Edge::E01, Edge::E04, Edge::E13, Edge::E23, Edge::E26, Edge::E45, Edge::E15, Edge::E57, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E57, Edge::E23, Edge::E26, Edge::E57, Edge::E45, Edge::E23, Edge::E45, Edge::E01, Edge::E23, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E23, Edge::E26, Edge::E57, Edge::E02, Edge::E23, Edge::E57, Edge::E02, Edge::E57, Edge::E45, Edge::E02, Edge::E45, Edge::E04, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E57, Edge::E45, Edge::E23, Edge::E02, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E37, Edge::E23, Edge::E01, Edge::E04, Edge::E37, Edge::E45, Edge::E15, Edge::E57, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E57, Edge::E45, Edge::E01, Edge::E13, Edge::E57, Edge::E23, Edge::E02, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E23, Edge::E13, Edge::E57, Edge::E23, Edge::E57, Edge::E04, Edge::E23, Edge::E04, Edge::E37, Edge::E45, Edge::E04, Edge::E57, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E26, Edge::E02, Edge::E37, Edge::E26, Edge::E13, Edge::E02, Edge::E15, Edge::E57, Edge::E45, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E45, Edge::E15, Edge::E57, Edge::E01, Edge::E04, Edge::E13, Edge::E04, Edge::E26, Edge::E13, Edge::E04, Edge::E37, Edge::E26, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E57, Edge::E45, Edge::E01, Edge::E57, Edge::E01, Edge::E37, Edge::E57, Edge::E37, Edge::E26, Edge::E37, Edge::E01, Edge::E02, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E57, Edge::E45, Edge::E04, Edge::E57, Edge::E04, Edge::E26, Edge::E26, Edge::E04, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E46, Edge::E04, Edge::E57, Edge::E46, Edge::E15, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E02, Edge::E01, Edge::E15, Edge::E57, Edge::E02, Edge::E57, Edge::E46, Edge::E02, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E46, Edge::E04, Edge::E01, Edge::E13, Edge::E46, Edge::E13, Edge::E57, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E57, Edge::E02, Edge::E02, Edge::E57, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E46, Edge::E04, Edge::E15, Edge::E57, Edge::E46, Edge::E26, Edge::E13, Edge::E23, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E26, Edge::E13, Edge::E23, Edge::E15, Edge::E57, Edge::E01, Edge::E57, Edge::E02, Edge::E01, Edge::E57, Edge::E46, Edge::E02, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E04, Edge::E01, Edge::E23, Edge::E04, Edge::E23, Edge::E57, Edge::E04, Edge::E57, Edge::E46, Edge::E26, Edge::E57, Edge::E23, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E23, Edge::E26, Edge::E57, Edge::E23, Edge::E57, Edge::E02, Edge::E02, Edge::E57, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E46, Edge::E15, Edge::E57, Edge::E46, Edge::E04, Edge::E15, Edge::E02, Edge::E37, Edge::E23, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E57, Edge::E46, Edge::E15, Edge::E46, Edge::E23, Edge::E15, Edge::E23, Edge::E01, Edge::E23, Edge::E46, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E23, Edge::E02, Edge::E37, Edge::E01, Edge::E13, Edge::E04, Edge::E13, Edge::E46, Edge::E04, Edge::E13, Edge::E57, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E37, Edge::E23, Edge::E13, Edge::E37, Edge::E13, Edge::E46, Edge::E46, Edge::E13, Edge::E57, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E57, Edge::E04, Edge::E04, Edge::E57, Edge::E46, Edge::E26, Edge::E13, Edge::E02, Edge::E26, Edge::E02, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E57, Edge::E46, Edge::E01, Edge::E57, Edge::E01, Edge::E15, Edge::E46, Edge::E37, Edge::E01, Edge::E13, Edge::E01, Edge::E26, Edge::E37, Edge::E26, Edge::E01, Edge::End}},
////    {{Edge::E37, Edge::E26, Edge::E01, Edge::E37, Edge::E01, Edge::E02, Edge::E26, Edge::E57, Edge::E01, Edge::E04, Edge::E01, Edge::E46, Edge::E57, Edge::E46, Edge::E01, Edge::End}},
////    {{Edge::E37, Edge::E26, Edge::E57, Edge::E46, Edge::E37, Edge::E57, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E26, Edge::E67, Edge::E57, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E04, Edge::E02, Edge::E57, Edge::E26, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E01, Edge::E13, Edge::E57, Edge::E26, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E04, Edge::E02, Edge::E13, Edge::E15, Edge::E04, Edge::E57, Edge::E26, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E67, Edge::E57, Edge::E23, Edge::E67, Edge::E13, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E67, Edge::E57, Edge::E13, Edge::E23, Edge::E67, Edge::E02, Edge::E01, Edge::E04, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E67, Edge::E57, Edge::E15, Edge::E01, Edge::E67, Edge::E01, Edge::E23, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E57, Edge::E15, Edge::E04, Edge::E57, Edge::E04, Edge::E23, Edge::E57, Edge::E23, Edge::E67, Edge::E02, Edge::E23, Edge::E04, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E23, Edge::E02, Edge::E37, Edge::E26, Edge::E67, Edge::E57, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E37, Edge::E01, Edge::E04, Edge::E37, Edge::E23, Edge::E01, Edge::E26, Edge::E67, Edge::E57, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E13, Edge::E15, Edge::E23, Edge::E02, Edge::E37, Edge::E57, Edge::E26, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E57, Edge::E26, Edge::E67, Edge::E13, Edge::E15, Edge::E23, Edge::E15, Edge::E37, Edge::E23, Edge::E15, Edge::E04, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E67, Edge::E02, Edge::E37, Edge::E67, Edge::E57, Edge::E02, Edge::E57, Edge::E13, Edge::E02, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E04, Edge::E37, Edge::E01, Edge::E37, Edge::E57, Edge::E01, Edge::E57, Edge::E13, Edge::E57, Edge::E37, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E02, Edge::E37, Edge::E67, Edge::E01, Edge::E02, Edge::E67, Edge::E01, Edge::E67, Edge::E57, Edge::E01, Edge::E57, Edge::E15, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E67, Edge::E57, Edge::E15, Edge::E67, Edge::E15, Edge::E37, Edge::E37, Edge::E15, Edge::E04, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E57, Edge::E26, Edge::E67, Edge::E45, Edge::E46, Edge::E04, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E45, Edge::E02, Edge::E01, Edge::E45, Edge::E46, Edge::E02, Edge::E67, Edge::E57, Edge::E26, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E15, Edge::E01, Edge::E57, Edge::E26, Edge::E67, Edge::E04, Edge::E45, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E26, Edge::E67, Edge::E57, Edge::E13, Edge::E15, Edge::E46, Edge::E13, Edge::E46, Edge::E02, Edge::E46, Edge::E15, Edge::E45, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E67, Edge::E13, Edge::E23, Edge::E67, Edge::E57, Edge::E13, Edge::E45, Edge::E46, Edge::E04, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E23, Edge::E57, Edge::E57, Edge::E23, Edge::E67, Edge::E02, Edge::E01, Edge::E45, Edge::E02, Edge::E45, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E04, Edge::E45, Edge::E46, Edge::E15, Edge::E01, Edge::E57, Edge::E01, Edge::E67, Edge::E57, Edge::E01, Edge::E23, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E46, Edge::E02, Edge::E15, Edge::E46, Edge::E15, Edge::E45, Edge::E02, Edge::E23, Edge::E15, Edge::E57, Edge::E15, Edge::E67, Edge::E23, Edge::E67, Edge::E15, Edge::End}},
////    {{Edge::E02, Edge::E37, Edge::E23, Edge::E46, Edge::E04, Edge::E45, Edge::E26, Edge::E67, Edge::E57, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E57, Edge::E26, Edge::E67, Edge::E45, Edge::E46, Edge::E23, Edge::E45, Edge::E23, Edge::E01, Edge::E23, Edge::E46, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E13, Edge::E15, Edge::E45, Edge::E46, Edge::E04, Edge::E23, Edge::E02, Edge::E37, Edge::E57, Edge::E26, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E23, Edge::E13, Edge::E15, Edge::E37, Edge::E23, Edge::E15, Edge::E45, Edge::E37, Edge::E46, Edge::E37, Edge::E45, Edge::E57, Edge::E26, Edge::E67, Edge::End}},
////    {{Edge::E04, Edge::E45, Edge::E46, Edge::E02, Edge::E37, Edge::E57, Edge::E02, Edge::E57, Edge::E13, Edge::E57, Edge::E37, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E57, Edge::E13, Edge::E37, Edge::E57, Edge::E37, Edge::E67, Edge::E13, Edge::E01, Edge::E37, Edge::E46, Edge::E37, Edge::E45, Edge::E01, Edge::E45, Edge::E37, Edge::End}},
////    {{Edge::E01, Edge::E57, Edge::E15, Edge::E01, Edge::E67, Edge::E57, Edge::E01, Edge::E02, Edge::E67, Edge::E37, Edge::E67, Edge::E02, Edge::E04, Edge::E45, Edge::E46, Edge::End}},
////    {{Edge::E67, Edge::E57, Edge::E15, Edge::E67, Edge::E15, Edge::E37, Edge::E45, Edge::E46, Edge::E15, Edge::E46, Edge::E37, Edge::E15, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E26, Edge::E45, Edge::E15, Edge::E67, Edge::E45, Edge::E26, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E45, Edge::E26, Edge::E67, Edge::E45, Edge::E15, Edge::E26, Edge::E01, Edge::E04, Edge::E02, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E26, Edge::E01, Edge::E13, Edge::E26, Edge::E67, Edge::E01, Edge::E67, Edge::E45, Edge::E01, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E04, Edge::E02, Edge::E13, Edge::E04, Edge::E13, Edge::E67, Edge::E04, Edge::E67, Edge::E45, Edge::E67, Edge::E13, Edge::E26, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E45, Edge::E15, Edge::E13, Edge::E23, Edge::E45, Edge::E23, Edge::E67, Edge::E45, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E02, Edge::E01, Edge::E04, Edge::E13, Edge::E23, Edge::E15, Edge::E23, Edge::E45, Edge::E15, Edge::E23, Edge::E67, Edge::E45, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E23, Edge::E45, Edge::E45, Edge::E23, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E04, Edge::E02, Edge::E23, Edge::E04, Edge::E23, Edge::E45, Edge::E45, Edge::E23, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E26, Edge::E45, Edge::E15, Edge::E26, Edge::E67, Edge::E45, Edge::E37, Edge::E23, Edge::E02, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E04, Edge::E23, Edge::E23, Edge::E04, Edge::E37, Edge::E45, Edge::E15, Edge::E26, Edge::E45, Edge::E26, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E02, Edge::E37, Edge::E23, Edge::E01, Edge::E13, Edge::E67, Edge::E01, Edge::E67, Edge::E45, Edge::E67, Edge::E13, Edge::E26, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E67, Edge::E45, Edge::E13, Edge::E67, Edge::E13, Edge::E26, Edge::E45, Edge::E04, Edge::E13, Edge::E23, Edge::E13, Edge::E37, Edge::E04, Edge::E37, Edge::E13, Edge::End}},
////    {{Edge::E15, Edge::E67, Edge::E45, Edge::E15, Edge::E02, Edge::E67, Edge::E15, Edge::E13, Edge::E02, Edge::E37, Edge::E67, Edge::E02, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E04, Edge::E37, Edge::E13, Edge::E04, Edge::E13, Edge::E01, Edge::E37, Edge::E67, Edge::E13, Edge::E15, Edge::E13, Edge::E45, Edge::E67, Edge::E45, Edge::E13, Edge::End}},
////    {{Edge::E02, Edge::E37, Edge::E67, Edge::E02, Edge::E67, Edge::E01, Edge::E01, Edge::E67, Edge::E45, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E67, Edge::E45, Edge::E04, Edge::E37, Edge::E67, Edge::E04, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E46, Edge::E26, Edge::E67, Edge::E46, Edge::E04, Edge::E26, Edge::E04, Edge::E15, Edge::E26, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E46, Edge::E02, Edge::E01, Edge::E26, Edge::E46, Edge::E01, Edge::E15, Edge::E26, Edge::E67, Edge::E46, Edge::E26, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E26, Edge::E67, Edge::E46, Edge::E13, Edge::E26, Edge::E46, Edge::E13, Edge::E46, Edge::E04, Edge::E13, Edge::E04, Edge::E01, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E26, Edge::E67, Edge::E46, Edge::E26, Edge::E46, Edge::E13, Edge::E13, Edge::E46, Edge::E02, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E23, Edge::E67, Edge::E13, Edge::E67, Edge::E04, Edge::E13, Edge::E04, Edge::E15, Edge::E04, Edge::E67, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E23, Edge::E67, Edge::E15, Edge::E23, Edge::E15, Edge::E13, Edge::E67, Edge::E46, Edge::E15, Edge::E01, Edge::E15, Edge::E02, Edge::E46, Edge::E02, Edge::E15, Edge::End}},
////    {{Edge::E46, Edge::E04, Edge::E01, Edge::E46, Edge::E01, Edge::E67, Edge::E67, Edge::E01, Edge::E23, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E46, Edge::E02, Edge::E23, Edge::E67, Edge::E46, Edge::E23, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E23, Edge::E02, Edge::E37, Edge::E26, Edge::E67, Edge::E04, Edge::E26, Edge::E04, Edge::E15, Edge::E04, Edge::E67, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E23, Edge::E01, Edge::E46, Edge::E23, Edge::E46, Edge::E37, Edge::E01, Edge::E15, Edge::E46, Edge::E67, Edge::E46, Edge::E26, Edge::E15, Edge::E26, Edge::E46, Edge::End}},
////    {{Edge::E13, Edge::E04, Edge::E01, Edge::E13, Edge::E46, Edge::E04, Edge::E13, Edge::E26, Edge::E46, Edge::E67, Edge::E46, Edge::E26, Edge::E23, Edge::E02, Edge::E37, Edge::End}},
////    {{Edge::E37, Edge::E23, Edge::E13, Edge::E37, Edge::E13, Edge::E46, Edge::E26, Edge::E67, Edge::E13, Edge::E67, Edge::E46, Edge::E13, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E04, Edge::E15, Edge::E67, Edge::E04, Edge::E67, Edge::E46, Edge::E15, Edge::E13, Edge::E67, Edge::E37, Edge::E67, Edge::E02, Edge::E13, Edge::E02, Edge::E67, Edge::End}},
////    {{Edge::E01, Edge::E15, Edge::E13, Edge::E37, Edge::E67, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E46, Edge::E04, Edge::E01, Edge::E46, Edge::E01, Edge::E67, Edge::E02, Edge::E37, Edge::E01, Edge::E37, Edge::E67, Edge::E01, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E46, Edge::E37, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E46, Edge::E67, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E02, Edge::E01, Edge::E04, Edge::E37, Edge::E46, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E13, Edge::E15, Edge::E37, Edge::E46, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E04, Edge::E13, Edge::E15, Edge::E04, Edge::E02, Edge::E13, Edge::E37, Edge::E46, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E26, Edge::E13, Edge::E23, Edge::E67, Edge::E37, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E23, Edge::E26, Edge::E02, Edge::E01, Edge::E04, Edge::E67, Edge::E37, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E23, Edge::E15, Edge::E01, Edge::E23, Edge::E26, Edge::E15, Edge::E67, Edge::E37, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E67, Edge::E37, Edge::E46, Edge::E23, Edge::E26, Edge::E02, Edge::E26, Edge::E04, Edge::E02, Edge::E26, Edge::E15, Edge::E04, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E46, Edge::E23, Edge::E02, Edge::E67, Edge::E23, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E46, Edge::E01, Edge::E04, Edge::E46, Edge::E67, Edge::E01, Edge::E67, Edge::E23, Edge::E01, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E23, Edge::E46, Edge::E67, Edge::E23, Edge::E02, Edge::E46, Edge::E01, Edge::E13, Edge::E15, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E67, Edge::E23, Edge::E13, Edge::E04, Edge::E67, Edge::E13, Edge::E15, Edge::E04, Edge::E04, Edge::E46, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E26, Edge::E46, Edge::E67, Edge::E26, Edge::E13, Edge::E46, Edge::E13, Edge::E02, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E26, Edge::E46, Edge::E67, Edge::E13, Edge::E46, Edge::E26, Edge::E13, Edge::E04, Edge::E46, Edge::E13, Edge::E01, Edge::E04, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E02, Edge::E46, Edge::E01, Edge::E46, Edge::E26, Edge::E01, Edge::E26, Edge::E15, Edge::E67, Edge::E26, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E46, Edge::E67, Edge::E26, Edge::E46, Edge::E26, Edge::E04, Edge::E04, Edge::E26, Edge::E15, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E67, Edge::E04, Edge::E45, Edge::E37, Edge::E04, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E02, Edge::E67, Edge::E37, Edge::E02, Edge::E01, Edge::E67, Edge::E01, Edge::E45, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E04, Edge::E67, Edge::E37, Edge::E04, Edge::E45, Edge::E67, Edge::E15, Edge::E01, Edge::E13, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E45, Edge::E67, Edge::E15, Edge::E67, Edge::E02, Edge::E15, Edge::E02, Edge::E13, Edge::E37, Edge::E02, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E67, Edge::E04, Edge::E45, Edge::E67, Edge::E37, Edge::E04, Edge::E23, Edge::E26, Edge::E13, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E23, Edge::E26, Edge::E02, Edge::E01, Edge::E37, Edge::E01, Edge::E67, Edge::E37, Edge::E01, Edge::E45, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E45, Edge::E37, Edge::E04, Edge::E45, Edge::E67, Edge::E37, Edge::E01, Edge::E23, Edge::E15, Edge::E23, Edge::E26, Edge::E15, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E26, Edge::E15, Edge::E02, Edge::E26, Edge::E02, Edge::E23, Edge::E15, Edge::E45, Edge::E02, Edge::E37, Edge::E02, Edge::E67, Edge::E45, Edge::E67, Edge::E02, Edge::End}},
////    {{Edge::E04, Edge::E23, Edge::E02, Edge::E04, Edge::E45, Edge::E23, Edge::E45, Edge::E67, Edge::E23, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E45, Edge::E23, Edge::E45, Edge::E67, Edge::E23, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E15, Edge::E01, Edge::E23, Edge::E02, Edge::E45, Edge::E23, Edge::E45, Edge::E67, Edge::E45, Edge::E02, Edge::E04, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E15, Edge::E45, Edge::E13, Edge::E45, Edge::E23, Edge::E23, Edge::E45, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E04, Edge::E13, Edge::E02, Edge::E04, Edge::E67, Edge::E13, Edge::E04, Edge::E45, Edge::E67, Edge::E67, Edge::E26, Edge::E13, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E26, Edge::E13, Edge::E01, Edge::E26, Edge::E01, Edge::E67, Edge::E67, Edge::E01, Edge::E45, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E45, Edge::E67, Edge::E02, Edge::E45, Edge::E02, Edge::E04, Edge::E67, Edge::E26, Edge::E02, Edge::E01, Edge::E02, Edge::E15, Edge::E26, Edge::E15, Edge::E02, Edge::End}},
////    {{Edge::E26, Edge::E15, Edge::E45, Edge::E67, Edge::E26, Edge::E45, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E45, Edge::E15, Edge::E57, Edge::E46, Edge::E67, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E04, Edge::E02, Edge::E45, Edge::E15, Edge::E57, Edge::E37, Edge::E46, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E57, Edge::E01, Edge::E13, Edge::E57, Edge::E45, Edge::E01, Edge::E46, Edge::E67, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E37, Edge::E46, Edge::E67, Edge::E04, Edge::E02, Edge::E45, Edge::E02, Edge::E57, Edge::E45, Edge::E02, Edge::E13, Edge::E57, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E57, Edge::E45, Edge::E26, Edge::E13, Edge::E23, Edge::E46, Edge::E67, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E67, Edge::E37, Edge::E46, Edge::E13, Edge::E23, Edge::E26, Edge::E01, Edge::E04, Edge::E02, Edge::E45, Edge::E15, Edge::E57, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E46, Edge::E67, Edge::E37, Edge::E57, Edge::E45, Edge::E26, Edge::E45, Edge::E23, Edge::E26, Edge::E45, Edge::E01, Edge::E23, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E02, Edge::E45, Edge::E04, Edge::E02, Edge::E57, Edge::E45, Edge::E02, Edge::E23, Edge::E57, Edge::E26, Edge::E57, Edge::E23, Edge::E37, Edge::E46, Edge::E67, Edge::End}},
////    {{Edge::E46, Edge::E23, Edge::E02, Edge::E46, Edge::E67, Edge::E23, Edge::E57, Edge::E45, Edge::E15, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E57, Edge::E45, Edge::E01, Edge::E04, Edge::E67, Edge::E01, Edge::E67, Edge::E23, Edge::E67, Edge::E04, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E02, Edge::E67, Edge::E23, Edge::E02, Edge::E46, Edge::E67, Edge::E13, Edge::E57, Edge::E01, Edge::E57, Edge::E45, Edge::E01, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E67, Edge::E23, Edge::E04, Edge::E67, Edge::E04, Edge::E46, Edge::E23, Edge::E13, Edge::E04, Edge::E45, Edge::E04, Edge::E57, Edge::E13, Edge::E57, Edge::E04, Edge::End}},
////    {{Edge::E15, Edge::E57, Edge::E45, Edge::E26, Edge::E13, Edge::E67, Edge::E13, Edge::E46, Edge::E67, Edge::E13, Edge::E02, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E67, Edge::E26, Edge::E13, Edge::E46, Edge::E67, Edge::E13, Edge::E01, Edge::E46, Edge::E04, Edge::E46, Edge::E01, Edge::E15, Edge::E57, Edge::E45, Edge::End}},
////    {{Edge::E45, Edge::E01, Edge::E26, Edge::E45, Edge::E26, Edge::E57, Edge::E01, Edge::E02, Edge::E26, Edge::E67, Edge::E26, Edge::E46, Edge::E02, Edge::E46, Edge::E26, Edge::End}},
////    {{Edge::E46, Edge::E67, Edge::E26, Edge::E46, Edge::E26, Edge::E04, Edge::E57, Edge::E45, Edge::E26, Edge::E45, Edge::E04, Edge::E26, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E67, Edge::E15, Edge::E57, Edge::E67, Edge::E37, Edge::E15, Edge::E37, Edge::E04, Edge::E15, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E02, Edge::E67, Edge::E37, Edge::E01, Edge::E67, Edge::E02, Edge::E01, Edge::E57, Edge::E67, Edge::E01, Edge::E15, Edge::E57, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E37, Edge::E04, Edge::E01, Edge::E57, Edge::E37, Edge::E01, Edge::E13, Edge::E57, Edge::E57, Edge::E67, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E67, Edge::E37, Edge::E02, Edge::E67, Edge::E02, Edge::E57, Edge::E57, Edge::E02, Edge::E13, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E23, Edge::E26, Edge::E15, Edge::E57, Edge::E37, Edge::E15, Edge::E37, Edge::E04, Edge::E37, Edge::E57, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E37, Edge::E02, Edge::E01, Edge::E67, Edge::E37, Edge::E01, Edge::E15, Edge::E67, Edge::E57, Edge::E67, Edge::E15, Edge::E13, Edge::E23, Edge::E26, Edge::End}},
////    {{Edge::E37, Edge::E04, Edge::E57, Edge::E37, Edge::E57, Edge::E67, Edge::E04, Edge::E01, Edge::E57, Edge::E26, Edge::E57, Edge::E23, Edge::E01, Edge::E23, Edge::E57, Edge::End}},
////    {{Edge::E67, Edge::E37, Edge::E02, Edge::E67, Edge::E02, Edge::E57, Edge::E23, Edge::E26, Edge::E02, Edge::E26, Edge::E57, Edge::E02, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E57, Edge::E04, Edge::E15, Edge::E57, Edge::E23, Edge::E04, Edge::E57, Edge::E67, Edge::E23, Edge::E02, Edge::E04, Edge::E23, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E57, Edge::E67, Edge::E15, Edge::E67, Edge::E01, Edge::E01, Edge::E67, Edge::E23, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E57, Edge::E04, Edge::E13, Edge::E04, Edge::E01, Edge::E57, Edge::E67, Edge::E04, Edge::E02, Edge::E04, Edge::E23, Edge::E67, Edge::E23, Edge::E04, Edge::End}},
////    {{Edge::E13, Edge::E57, Edge::E67, Edge::E23, Edge::E13, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E02, Edge::E67, Edge::E13, Edge::E67, Edge::E26, Edge::E02, Edge::E04, Edge::E67, Edge::E57, Edge::E67, Edge::E15, Edge::E04, Edge::E15, Edge::E67, Edge::End}},
////    {{Edge::E26, Edge::E13, Edge::E01, Edge::E26, Edge::E01, Edge::E67, Edge::E15, Edge::E57, Edge::E01, Edge::E57, Edge::E67, Edge::E01, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E02, Edge::E04, Edge::E57, Edge::E67, Edge::E26, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E26, Edge::E57, Edge::E67, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E37, Edge::E57, Edge::E26, Edge::E46, Edge::E57, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E37, Edge::E57, Edge::E26, Edge::E37, Edge::E46, Edge::E57, Edge::E04, Edge::E02, Edge::E01, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E57, Edge::E37, Edge::E46, Edge::E57, Edge::E26, Edge::E37, Edge::E13, Edge::E15, Edge::E01, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E26, Edge::E46, Edge::E57, Edge::E26, Edge::E37, Edge::E46, Edge::E15, Edge::E04, Edge::E13, Edge::E04, Edge::E02, Edge::E13, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E37, Edge::E13, Edge::E23, Edge::E37, Edge::E46, Edge::E13, Edge::E46, Edge::E57, Edge::E13, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E04, Edge::E02, Edge::E13, Edge::E23, Edge::E46, Edge::E13, Edge::E46, Edge::E57, Edge::E46, Edge::E23, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E46, Edge::E57, Edge::E15, Edge::E23, Edge::E46, Edge::E15, Edge::E01, Edge::E23, Edge::E23, Edge::E37, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E46, Edge::E57, Edge::E23, Edge::E46, Edge::E23, Edge::E37, Edge::E57, Edge::E15, Edge::E23, Edge::E02, Edge::E23, Edge::E04, Edge::E15, Edge::E04, Edge::E23, Edge::End}},
////    {{Edge::E23, Edge::E57, Edge::E26, Edge::E23, Edge::E02, Edge::E57, Edge::E02, Edge::E46, Edge::E57, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E04, Edge::E23, Edge::E01, Edge::E04, Edge::E57, Edge::E23, Edge::E04, Edge::E46, Edge::E57, Edge::E26, Edge::E23, Edge::E57, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E01, Edge::E13, Edge::E57, Edge::E26, Edge::E02, Edge::E57, Edge::E02, Edge::E46, Edge::E02, Edge::E26, Edge::E23, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E04, Edge::E23, Edge::E15, Edge::E23, Edge::E13, Edge::E04, Edge::E46, Edge::E23, Edge::E26, Edge::E23, Edge::E57, Edge::E46, Edge::E57, Edge::E23, Edge::End}},
////    {{Edge::E13, Edge::E02, Edge::E57, Edge::E02, Edge::E46, Edge::E57, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E04, Edge::E46, Edge::E01, Edge::E46, Edge::E13, Edge::E13, Edge::E46, Edge::E57, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E01, Edge::E02, Edge::E15, Edge::E02, Edge::E57, Edge::E57, Edge::E02, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E04, Edge::E46, Edge::E57, Edge::E15, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E57, Edge::E04, Edge::E45, Edge::E57, Edge::E26, Edge::E04, Edge::E26, Edge::E37, Edge::E04, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E57, Edge::E01, Edge::E45, Edge::E57, Edge::E37, Edge::E01, Edge::E57, Edge::E26, Edge::E37, Edge::E37, Edge::E02, Edge::E01, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E13, Edge::E15, Edge::E04, Edge::E45, Edge::E26, Edge::E04, Edge::E26, Edge::E37, Edge::E26, Edge::E45, Edge::E57, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E26, Edge::E37, Edge::E45, Edge::E26, Edge::E45, Edge::E57, Edge::E37, Edge::E02, Edge::E45, Edge::E15, Edge::E45, Edge::E13, Edge::E02, Edge::E13, Edge::E45, Edge::End}},
////    {{Edge::E23, Edge::E57, Edge::E13, Edge::E23, Edge::E04, Edge::E57, Edge::E23, Edge::E37, Edge::E04, Edge::E45, Edge::E57, Edge::E04, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E45, Edge::E37, Edge::E01, Edge::E37, Edge::E02, Edge::E45, Edge::E57, Edge::E37, Edge::E23, Edge::E37, Edge::E13, Edge::E57, Edge::E13, Edge::E37, Edge::End}},
////    {{Edge::E01, Edge::E23, Edge::E57, Edge::E01, Edge::E57, Edge::E15, Edge::E23, Edge::E37, Edge::E57, Edge::E45, Edge::E57, Edge::E04, Edge::E37, Edge::E04, Edge::E57, Edge::End}},
////    {{Edge::E15, Edge::E45, Edge::E57, Edge::E23, Edge::E37, Edge::E02, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E23, Edge::E57, Edge::E26, Edge::E02, Edge::E57, Edge::E23, Edge::E02, Edge::E45, Edge::E57, Edge::E02, Edge::E04, Edge::E45, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E57, Edge::E26, Edge::E23, Edge::E57, Edge::E23, Edge::E45, Edge::E45, Edge::E23, Edge::E01, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E02, Edge::E26, Edge::E23, Edge::E02, Edge::E57, Edge::E26, Edge::E02, Edge::E04, Edge::E57, Edge::E45, Edge::E57, Edge::E04, Edge::E01, Edge::E13, Edge::E15, Edge::End}},
////    {{Edge::E57, Edge::E26, Edge::E23, Edge::E57, Edge::E23, Edge::E45, Edge::E13, Edge::E15, Edge::E23, Edge::E15, Edge::E45, Edge::E23, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E04, Edge::E45, Edge::E57, Edge::E04, Edge::E57, Edge::E02, Edge::E02, Edge::E57, Edge::E13, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E45, Edge::E57, Edge::E13, Edge::E01, Edge::E57, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E04, Edge::E45, Edge::E57, Edge::E04, Edge::E57, Edge::E02, Edge::E15, Edge::E01, Edge::E57, Edge::E01, Edge::E02, Edge::E57, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E45, Edge::E57, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E45, Edge::E37, Edge::E46, Edge::E45, Edge::E15, Edge::E37, Edge::E15, Edge::E26, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E04, Edge::E02, Edge::E45, Edge::E15, Edge::E46, Edge::E15, Edge::E37, Edge::E46, Edge::E15, Edge::E26, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E26, Edge::E37, Edge::E13, Edge::E37, Edge::E45, Edge::E13, Edge::E45, Edge::E01, Edge::E46, Edge::E45, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E02, Edge::E13, Edge::E45, Edge::E02, Edge::E45, Edge::E04, Edge::E13, Edge::E26, Edge::E45, Edge::E46, Edge::E45, Edge::E37, Edge::E26, Edge::E37, Edge::E45, Edge::End}},
////    {{Edge::E45, Edge::E37, Edge::E46, Edge::E15, Edge::E37, Edge::E45, Edge::E15, Edge::E23, Edge::E37, Edge::E15, Edge::E13, Edge::E23, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E46, Edge::E45, Edge::E15, Edge::E37, Edge::E46, Edge::E15, Edge::E13, Edge::E37, Edge::E23, Edge::E37, Edge::E13, Edge::E01, Edge::E04, Edge::E02, Edge::End}},
////    {{Edge::E37, Edge::E46, Edge::E45, Edge::E37, Edge::E45, Edge::E23, Edge::E23, Edge::E45, Edge::E01, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E37, Edge::E46, Edge::E45, Edge::E37, Edge::E45, Edge::E23, Edge::E04, Edge::E02, Edge::E45, Edge::E02, Edge::E23, Edge::E45, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E23, Edge::E15, Edge::E26, Edge::E23, Edge::E46, Edge::E15, Edge::E23, Edge::E02, Edge::E46, Edge::E46, Edge::E45, Edge::E15, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E26, Edge::E46, Edge::E15, Edge::E46, Edge::E45, Edge::E26, Edge::E23, Edge::E46, Edge::E04, Edge::E46, Edge::E01, Edge::E23, Edge::E01, Edge::E46, Edge::End}},
////    {{Edge::E02, Edge::E46, Edge::E26, Edge::E02, Edge::E26, Edge::E23, Edge::E46, Edge::E45, Edge::E26, Edge::E13, Edge::E26, Edge::E01, Edge::E45, Edge::E01, Edge::E26, Edge::End}},
////    {{Edge::E13, Edge::E26, Edge::E23, Edge::E04, Edge::E46, Edge::E45, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E45, Edge::E15, Edge::E13, Edge::E45, Edge::E13, Edge::E46, Edge::E46, Edge::E13, Edge::E02, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E45, Edge::E15, Edge::E13, Edge::E45, Edge::E13, Edge::E46, Edge::E01, Edge::E04, Edge::E13, Edge::E04, Edge::E46, Edge::E13, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E45, Edge::E01, Edge::E02, Edge::E46, Edge::E45, Edge::E02, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E45, Edge::E04, Edge::E46, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E26, Edge::E04, Edge::E26, Edge::E37, Edge::E04, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E02, Edge::E01, Edge::E15, Edge::E02, Edge::E15, Edge::E37, Edge::E37, Edge::E15, Edge::E26, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E13, Edge::E26, Edge::E01, Edge::E26, Edge::E04, Edge::E04, Edge::E26, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E02, Edge::E13, Edge::E26, Edge::E37, Edge::E02, Edge::E26, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E23, Edge::E37, Edge::E13, Edge::E37, Edge::E15, Edge::E15, Edge::E37, Edge::E04, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E02, Edge::E01, Edge::E15, Edge::E02, Edge::E15, Edge::E37, Edge::E13, Edge::E23, Edge::E15, Edge::E23, Edge::E37, Edge::E15, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E23, Edge::E37, Edge::E04, Edge::E01, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E02, Edge::E23, Edge::E37, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E23, Edge::E02, Edge::E04, Edge::E23, Edge::E04, Edge::E26, Edge::E26, Edge::E04, Edge::E15, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E15, Edge::E26, Edge::E23, Edge::E01, Edge::E15, Edge::E23, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E23, Edge::E02, Edge::E04, Edge::E23, Edge::E04, Edge::E26, Edge::E01, Edge::E13, Edge::E04, Edge::E13, Edge::E26, Edge::E04, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E26, Edge::E23, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E13, Edge::E02, Edge::E04, Edge::E15, Edge::E13, Edge::E04, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E15, Edge::E13, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::E01, Edge::E02, Edge::E04, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////    {{Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End, Edge::End}},
////} };
////
////// Runtime check helpers:
////
////bool hasCanonicalRep(VertexMask mask) {
////    for (uint8_t cid = 0; cid < canonicalVertexMasks.size(); ++cid) {
////        for (uint8_t rid = 0; rid < vertexPermutationsTable.size(); ++rid) {
////            if (rotatedVertex(canonicalVertexMasks[cid], rid) == mask) {
////                return true;          // found a rep+rotation that produces m
////            }
////        }
////    }
////    return false;
////}
////
////std::pair<std::size_t, std::size_t> findBadRotation() {
////    for (std::size_t i = 0; i < canonicalVertexMasks.size(); ++i) {
////        for (std::size_t r = 0; r < vertexPermutationsTable.size(); ++r) {
////            if (!hasCanonicalRep(rotatedVertex(canonicalVertexMasks[i], r))) {
////                return { i, r };
////            }
////        }
////    }
////    return { std::numeric_limits<std::size_t>::max(), std::numeric_limits<std::size_t>::max() };
////}
////
////void debugFindMissing() {
////    for (uint16_t m = 0; m < 256; ++m) {
////        if (std::popcount(m) > 4) continue;
////        VertexMask vm{ static_cast<uint8_t>(m) };
////        bool covered = false;
////        for (size_t i = 0; i < canonicalVertexMasks.size() && !covered; ++i) {
////            for (size_t r = 0; r < vertexPermutationsTable.size(); ++r) {
////                if (rotatedVertex(canonicalVertexMasks[i], r) == vm) {
////                    covered = true;
////                    break;
////                }
////            }
////        }
////        if (!covered) {
////            std::bitset<8> b(m);
////            std::cerr << ">>> missing reducedMask = " << b
////                << " (decimal " << m << ")\n";
////            return;
////        }
////    }
////    std::cerr << "All reduced masks covered!\n";
////}
////
//
//INCLUDE_ENUM_OSTREAM;
//
//template<typename T, std::size_t Rows, std::size_t Cols>
//bool compareAndReport(
//    const std::array<std::array<T, Cols>, Rows>& a,
//    const std::array<std::array<T, Cols>, Rows>& b
//) {
//    for (std::size_t i = 0; i < Rows; ++i) {
//        for (std::size_t j = 0; j < Cols; ++j) {
//            if (a[i][j] != b[i][j]) {
//                std::cerr << "Mismatch at [" << i << "][" << j
//                    << "]: built=" << static_cast<int>(a[i][j])
//                    << " ref=" << static_cast<int>(b[i][j])
//                    << "\n";
//                return false;
//            }
//        }
//    }
//    return true;
//}
//
//template<typename T, std::size_t Rows, std::size_t Cols>
//bool compareAndReportSets(
//    const std::array<std::array<T, Cols>, Rows>& a,
//    const std::array<std::array<T, Cols>, Rows>& b
//) {
//    // Helper: turn one "raw" row into a canonical flattened vector
//    auto normalize = [&](const std::array<T, Cols>& row) {
//        // 1) pull out triples until we hit the End sentinel
//        std::vector<std::array<T, 3>> tris;
//        for (size_t i = 0; i < Cols; i += 3) {
//            if (row[i] == ::marching_cubes::utils::marching_cubes::Edge::End)
//                break;
//            tris.push_back({ row[i], row[i + 1], row[i + 2] });
//        }
//        // 2) sort each triple of edges
//        for (auto& tri : tris) {
//            std::sort(tri.begin(), tri.end());
//        }
//        // 3) sort the list of triples lexicographically
//        std::sort(tris.begin(), tris.end());
//        // 4) flatten back into a vector<T>
//        std::vector<T> flat;
//        flat.reserve(tris.size() * 3);
//        for (auto& tri : tris) {
//            flat.insert(flat.end(), tri.begin(), tri.end());
//        }
//        return flat;
//        };
//
//    // Build sets of normalized rows
//    std::set<std::vector<T>> setA, setB;
//    for (auto const& row : a) setA.insert(normalize(row));
//    for (auto const& row : b) setB.insert(normalize(row));
//
//    // Now compare just as before
//    std::size_t diffA = 0, diffB = 0;
//    bool identical = true;
//
//    // A \\ B
//    for (auto const& row : setA) {
//        if (setB.find(row) == setB.end()) {
//            std::cerr << "Only in first: [ ";
//            for (auto const& x : row) std::cerr << x << ' ';
//            std::cerr << "]\n";
//            identical = false;
//            ++diffA;
//        }
//    }
//    // B \\ A
//    for (auto const& row : setB) {
//        if (setA.find(row) == setA.end()) {
//            std::cerr << "Only in second: [ ";
//            for (auto const& x : row) std::cerr << x << ' ';
//            std::cerr << "]\n";
//            identical = false;
//            ++diffB;
//        }
//    }
//
//    if (diffA == 0 && diffB == 0) {
//        std::cout << "Both sets are identical.\n";
//    }
//    else {
//        std::cout << "Differences found: "
//            << diffA << " in first, "
//            << diffB << " in second.\n";
//    }
//
//    return identical;
//}
//
//template<typename T, std::size_t N>
//std::ostream& operator<<(std::ostream& os, const std::array<T, N>& arr)
//{
//    os << "{ ";
//    for (const auto& elem : arr) {
//        os << elem << ", ";
//    }
//    os << "}\n";
//	return os;
//}
//
//int main()
//{
//    /*static_assert(
//        ::marching_cubes::utils::marching_cubes::triTable == ::marching_cubes::utils::marching_cubes::makeTriTable(),
//        "triTable mismatch: your hard-coded table does not match the computed one"
//        );*/
//	std::cout << ::marching_cubes::utils::marching_cubes::canonicalVertexMasks << '\n';
//    auto triTable = ::marching_cubes::utils::marching_cubes::makeTriTable<c_WindingConvention>();
//	std::cout << "Built Triangle Table:\n" << triTable;
//    std::cout << "Ref Triangle Table:\n" << ::marching_cubes::utils::marching_cubes::triTable;
//    compareAndReportSets(triTable, ::marching_cubes::utils::marching_cubes::triTable);
//    return 0;
//}
