#pragma once
#ifndef MARCHING_CUBES_SCENE_CHUNK_CACHE_HPP
#define MARCHING_CUBES_SCENE_CHUNK_CACHE_HPP

#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan_core.h>

#include <atomic>
#include <tuple>

#include <core/aliases.hpp>
#include <utils/lru_cache.hpp>
#include <utils/morton.hpp>

namespace marching_cubes::scene {

	using ChunkId = glm::i64vec3;

	struct ChunkIdLess final {
		constexpr bool operator()(const ChunkId& a, const ChunkId& b) const noexcept
		{
			return std::tuple{ a.x, a.y, a.z } < std::tuple{ b.x, b.y, b.z };
		}
	};

	enum class ChunkState : u8 {
		Dirty,
		Generating,
		Generated,
		Clean,
		Evicted,
	};

	struct ChunkInfo final {
		VkDeviceSize vertexOffset{};
		u32 vertexCount{};
		VkDeviceSize indexOffset{};
		u32 indexCount{};
		std::atomic<ChunkState> state{ ChunkState::Dirty };
		std::atomic<u64> version{ 0 };

		ChunkInfo() noexcept = default;

		ChunkInfo(
			VkDeviceSize vertexOffset,
			u32 vertexCount,
			VkDeviceSize indexOffset,
			u32 indexCount,
			ChunkState state,
			u64 version = 0
		)
			: vertexOffset{ vertexOffset },
			vertexCount{ vertexCount },
			indexOffset{ indexOffset },
			indexCount{ indexCount },
			state{ state },
			version{ version }
		{
		}

		ChunkInfo(const ChunkInfo& other) noexcept
			: vertexOffset{ other.vertexOffset },
			vertexCount{ other.vertexCount },
			indexOffset{ other.indexOffset },
			indexCount{ other.indexCount },
			state{ other.state.load(std::memory_order_relaxed) },
			version{ other.version.load(std::memory_order_relaxed) }
		{
		}

		ChunkInfo& operator=(const ChunkInfo& other) noexcept
		{
			if (this != &other) {
				vertexOffset = other.vertexOffset;
				vertexCount = other.vertexCount;
				indexOffset = other.indexOffset;
				indexCount = other.indexCount;
				state.store(other.state.load(std::memory_order_relaxed), std::memory_order_relaxed);
				version.store(other.version.load(std::memory_order_relaxed), std::memory_order_relaxed);
			}
			return *this;
		}

		ChunkInfo(ChunkInfo&& other) noexcept
			: vertexOffset{ other.vertexOffset },
			vertexCount{ other.vertexCount },
			indexOffset{ other.indexOffset },
			indexCount{ other.indexCount },
			state{ other.state.load(std::memory_order_relaxed) },
			version{ other.version.load(std::memory_order_relaxed) }
		{
		}

		ChunkInfo& operator=(ChunkInfo&& other) noexcept
		{
			if (this != &other) {
				vertexOffset = other.vertexOffset;
				vertexCount = other.vertexCount;
				indexOffset = other.indexOffset;
				indexCount = other.indexCount;
				state.store(other.state.load(std::memory_order_relaxed), std::memory_order_relaxed);
				version.store(other.version.load(std::memory_order_relaxed), std::memory_order_relaxed);
			}
			return *this;
		}
	};

	using ChunkCache = utils::lru_cache::BasicLRUCache<ChunkId, ChunkInfo>;
}

#endif // !MARCHING_CUBES_SCENE_CHUNK_CACHE_HPP

