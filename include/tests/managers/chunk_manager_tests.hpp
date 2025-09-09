#pragma once
#ifndef MARCHING_CUBES_TESTS_MANAGERS_CHUNK_MANAGER_TESTS_HPP
#define MARCHING_CUBES_TESTS_MANAGERS_CHUNK_MANAGER_TESTS_HPP

#include <cassert>
#include <print>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

#include <allocators/chunk_allocator.hpp>
#include <camera/camera_first_person.hpp>
#include <core/aliases.hpp>
#include <managers/chunk_manager.hpp>
#include <scene/vertex.hpp>


namespace marching_cubes::tests {

    using glm::u64vec3;
	using scene::VertexPacking;
    using scene::BasicVertex;
    using scene::Position;
    using scene::Normal;
	using scene::Texcoord;
	using scene::Color;

    struct DummyBuffer {
        std::size_t getSize() const noexcept { return 1ull; }
        void* getBuffer() const noexcept { return nullptr; }
		void updateSize(VkDeviceSize) {}
    };

    using IndexT = u32;

    using DummyChunkManager = marching_cubes::managers::BasicChunkManager<
        BasicVertex<VertexPacking::Standard, Position>,
        DummyBuffer,
        IndexT,
        DummyBuffer
    >;

    struct GenResult {
        u32 vertexCount;
        u32 indexCount;
    };

    struct DrawStats {
        u32 draws = 0;
    };

    // Runs all tests in sequence
    void run_chunk_manager_tests();
}

#endif // !MARCHING_CUBES_TESTS_MANAGERS_CHUNK_MANAGER_TESTS_HPP
