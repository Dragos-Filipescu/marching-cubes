#pragma once
#ifndef MARCHING_CUBES_MANAGERS_CHUNK_MANAGER_HPP
#define MARCHING_CUBES_MANAGERS_CHUNK_MANAGER_HPP

//#define ENABLE_DEBUG_PRINT

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cmath>
#include <concepts>
#include <execution>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <print>
#include <span>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <allocators/chunk_allocator.hpp>
#include <camera/camera.hpp>
#include <collisions/collisions.hpp>
#include <core/aliases.hpp>
#include <camera/frustum.hpp>
#include <resources/object_buffer.hpp>
#include <scene/chunk_cache.hpp>
#include <scene/vertex.hpp>
#include <utils/debug_print.hpp>
#include <utils/morton.hpp>
#include <utils/type_safety.hpp>
#include <utils/utils.hpp>

#define DEBUG_PRINT_CHUNK_MANAGER(Fmt, ...) DEBUG_PRINT("ChunkManager", Fmt, __VA_ARGS__)

namespace marching_cubes::managers {

	using allocators::BufferLike;
	using allocators::BasicChunkAllocator;
	using camera::Camera;
	using camera::Frustum;
	using collisions::AABB;
	using resources::StagedObjectBuffer;
	using scene::ChunkCache;
	using scene::ChunkId;
	using scene::ChunkIdLess;
	using scene::ChunkInfo;
	using scene::ChunkState;
	using scene::Vertex;
	using utils::morton::Morton3T;
	using utils::morton::encode3D;
	using utils::morton::decode3D_x;
	using utils::morton::decode3D_y;
	using utils::morton::decode3D_z;

	namespace detail {

		template<typename T>
		concept DispatchGenerateMesh = std::invocable<
			T,
			const ChunkId&,
			std::shared_ptr<ChunkInfo>
		>;

		template<typename T>
		concept DispatchDrawChunk = std::invocable<
			T,
			const ChunkId&,
			std::shared_ptr<ChunkInfo>
		>;
	}

	template<
		Vertex VertexT,
		BufferLike VertexAllocatorBufferT,
		std::unsigned_integral IndexT,
		BufferLike IndexAllocatorBufferT
	>
		requires (
			std::default_initializable<VertexAllocatorBufferT>
			&& std::default_initializable<IndexAllocatorBufferT>
			&& std::move_constructible<VertexAllocatorBufferT>
			&& std::move_constructible<IndexAllocatorBufferT>
		)
	class BasicChunkManager final {

		using ChunkMapType = std::unordered_map<ChunkId, std::size_t>;
		using ChunkVecType = std::vector<std::size_t>;

		static constexpr i64 kAllocatorShellLayers = 2;

	public:

		using VertexType = VertexT;
		using VertexBufferType = VertexAllocatorBufferT;
		using IndexType = IndexT;
		using IndexBufferType = IndexAllocatorBufferT;

		struct MapIndexer final {
			const ChunkMapType* map{};

			MapIndexer() noexcept = default;

			MapIndexer(const ChunkMapType* map)
				: map{ map }
			{
			}

			std::size_t operator()(const ChunkId& chunkId) const
			{
				return map->at(chunkId);
			}
		};

		using IndexerType = MapIndexer;

		using ChunkAllocatorType = BasicChunkAllocator<
			VertexType,
			VertexAllocatorBufferT,
			IndexType,
			IndexAllocatorBufferT,
			glm::i64vec3,
			IndexerType
		>;

		using ChunkResolution = utils::StrongIntegral<struct ChunkResolutionTag, u64>;
		using VoxelSize = utils::StrongIntegral<struct VoxelSizeTag, u64>;
		using ChunkSize = utils::StrongIntegral<struct ChunkSizeTag, u64>;

		struct ChunkResult {
			u32 vertexCount{};
			u32 indexCount{};
		};

		BasicChunkManager() noexcept = default;

		BasicChunkManager(
			const Camera* camera,
			ChunkResolution chunkResolution,
			VoxelSize voxelSize,
			VertexAllocatorBufferT&& vertexBuffer,
			IndexAllocatorBufferT&& indexBuffer
		)
			: m_Camera{ camera },
			m_BasePos{ m_Camera->getTransform().getPosition() },
			m_BaseRot{ m_Camera->getTransform().getRotation() },
			m_ChunkResolution{ chunkResolution },
			m_VoxelSize{ voxelSize },
			m_ChunkSize{ m_ChunkResolution * m_VoxelSize },
			m_ChunkMap{ computeChunkMap() },
			m_ChunkCache{
				m_ChunkMap.size(),
				[](std::shared_ptr<ChunkInfo> info) {
					info->state.store(ChunkState::Evicted, std::memory_order_release);
				}
			},
			m_VisibleChunks{},
			m_DirtyChunks{},
			m_ChunkAllocator{
				getVerticesPerChunk(chunkResolution),
				std::forward<VertexAllocatorBufferT>(vertexBuffer),
				IndexerType{ &m_ChunkMap },
				getIndicesPerChunk(chunkResolution),
				std::forward<IndexAllocatorBufferT>(indexBuffer),
				IndexerType{ &m_ChunkMap },
			}
		{
			m_DirtyChunks.reserve(m_ChunkMap.size());
		}

		BasicChunkManager(const BasicChunkManager&) = delete;
		BasicChunkManager& operator=(const BasicChunkManager&) = delete;

		BasicChunkManager(BasicChunkManager&& other) noexcept
			: m_Camera{ std::exchange(other.m_Camera, nullptr) },
			m_BasePos{ other.m_BasePos },
			m_BaseRot{ other.m_BaseRot },
			m_ChunkResolution{ other.m_ChunkResolution },
			m_VoxelSize{ other.m_VoxelSize },
			m_ChunkSize{ ChunkSize{ m_ChunkResolution * m_VoxelSize } },
			m_ChunkMap{ std::move(other.m_ChunkMap) },
			m_ChunkCache{ std::move(other.m_ChunkCache) },
			m_VisibleChunks{ std::move(other.m_VisibleChunks) },
			m_DirtyChunks{ std::move(other.m_DirtyChunks) },
			m_ChunkAllocator{ std::move(other.m_ChunkAllocator) }
		{
			m_ChunkAllocator.rebindIndexers(
				IndexerType{ &m_ChunkMap },
				IndexerType{ &m_ChunkMap }
			);
		}
		BasicChunkManager& operator=(BasicChunkManager&& other) noexcept
		{
			if (this != &other) {
				m_Camera = std::exchange(other.m_Camera, nullptr);
				m_BasePos = std::move(other.m_BasePos);
				m_BaseRot = std::move(other.m_BaseRot);
				m_ChunkResolution = other.m_ChunkResolution;
				m_VoxelSize = other.m_VoxelSize;
				m_ChunkSize = ChunkSize{ m_ChunkResolution * m_VoxelSize };
				m_ChunkMap = std::move(other.m_ChunkMap);
				m_ChunkCache = std::move(other.m_ChunkCache);
				m_VisibleChunks = std::move(other.m_VisibleChunks);
				m_DirtyChunks = std::move(other.m_DirtyChunks);
				m_ChunkAllocator = std::move(other.m_ChunkAllocator);
			}

			m_ChunkAllocator.rebindIndexers(
				IndexerType{ &m_ChunkMap },
				IndexerType{ &m_ChunkMap }
			);

			return *this;
		}

		~BasicChunkManager() = default;

		[[nodiscard]] static VkDeviceSize ComputeVertexBufferSize(
			const Camera* camera,
			ChunkResolution chunkResolution,
			VoxelSize voxelSize
		)
		{
			return (
				static_cast<VkDeviceSize>(
					ComputeChunkCount(
						camera,
						ChunkSize{ chunkResolution * voxelSize }
					)
				)
				* getVerticesPerChunk(chunkResolution)
				* sizeof(VertexT)
			);
		}

		[[nodiscard]] static VkDeviceSize ComputeIndexBufferSize(
			const Camera* camera,
			ChunkResolution chunkResolution,
			VoxelSize voxelSize
		)
		{
			return (
				static_cast<VkDeviceSize>(
					ComputeChunkCount(
						camera,
						ChunkSize{ chunkResolution * voxelSize }
					)
				)
				* getIndicesPerChunk(chunkResolution)
				* sizeof(IndexT)
			);
		}

		[[nodiscard]] u64 getChunkResolution() const noexcept
		{
			return m_ChunkResolution;
		}

		[[nodiscard]] u64 getVoxelSize() const noexcept
		{
			return m_VoxelSize;
		}

		[[nodiscard]] u64 getChunkSize() const noexcept
		{
			return m_ChunkSize;
		}

		[[nodiscard]] const auto& getVisibleChunks() const noexcept
		{
			return m_VisibleChunks;
		}

		[[nodiscard]] const auto& getDirtyChunks() const noexcept
		{
			return m_DirtyChunks;
		}

		[[nodiscard]] const auto& getVertexBuffer() const noexcept
		{
			return m_ChunkAllocator.getVertexBuffer();
		}

		[[nodiscard]] auto& getVertexBuffer() noexcept
		{
			return m_ChunkAllocator.getVertexBuffer();
		}

		[[nodiscard]] const auto& getIndexBuffer() const noexcept
		{
			return m_ChunkAllocator.getIndexBuffer();
		}

		[[nodiscard]] auto& getIndexBuffer() noexcept
		{
			return m_ChunkAllocator.getIndexBuffer();
		}

		[[nodiscard]] auto getChunkInfo(const ChunkId& chunkId) noexcept
		{
			return m_ChunkCache.get(chunkId);
		}

		[[nodiscard]] auto peekChunkInfo(const ChunkId& chunkId) noexcept
		{
			return m_ChunkCache.peek(chunkId);
		}

		BasicChunkManager& update()
		{			
			m_VisibleChunks = getCandidates(m_Camera, m_ChunkSize);
			updateDirtyChunks();
			return *this;
		}

		template<detail::DispatchGenerateMesh DispatchGenerateMeshT>
		BasicChunkManager& generate(DispatchGenerateMeshT&& generateChunkCallback)
		{
			auto& callback = generateChunkCallback;
			for (const auto& cid : m_DirtyChunks) {
				auto info = m_ChunkCache.get(cid);
				if (!info || info->state.load(std::memory_order_acquire) != ChunkState::Dirty) {
					continue;
				}
				info->state.store(ChunkState::Generating, std::memory_order_release);
				std::invoke(callback, cid, info);
			}
			return *this;
		}

		template<detail::DispatchDrawChunk DispatchDrawChunkT>
		BasicChunkManager& draw(DispatchDrawChunkT&& drawChunkCallback)
		{
			auto& callback = drawChunkCallback;
			for (const auto& cid : m_VisibleChunks) {
				const auto info = m_ChunkCache.get(cid);
				if (!info || info->state.load(std::memory_order_acquire) != ChunkState::Clean) {
					continue;
				}
				std::invoke(callback, cid, info);
			}
			return *this;
		}

		void rebuild(const Camera* camera)
		{
			m_Camera = camera;
			m_ChunkMap = computeChunkMap();
			m_ChunkAllocator.rebindIndexers(
				IndexerType{ &m_ChunkMap },
				IndexerType{ &m_ChunkMap }
			);
			auto vertexBufferSize = ComputeVertexBufferSize(camera, m_ChunkResolution, m_VoxelSize);
			auto indexBufferSize = ComputeIndexBufferSize(camera, m_ChunkResolution, m_VoxelSize);
			std::println(
				"Updated chunk vertex buffer size: {:L} elements, {:L} MB",
				vertexBufferSize / sizeof(VertexT),
				vertexBufferSize / (1024ull * 1024ull)
			);
			std::println(
				"Updated chunk index buffer size: {:L} elements, {:L} MB",
				indexBufferSize / sizeof(IndexT),
				indexBufferSize / (1024ull * 1024ull)
			);
			m_ChunkAllocator.getVertexBuffer().updateSize(vertexBufferSize);
			m_ChunkAllocator.getIndexBuffer().updateSize(indexBufferSize);
			m_ChunkCache = ChunkCache{
				m_ChunkMap.size(),
				[](std::shared_ptr<ChunkInfo> info) {
					info->state.store(ChunkState::Evicted, std::memory_order_release);
				}
			};
			m_VisibleChunks.clear();
			m_DirtyChunks.clear();
			m_BasePos = m_Camera->getTransform().getPosition();
			m_BaseRot = m_Camera->getTransform().getRotation();
		}

	private:

		[[nodiscard]] static constexpr u64 getVerticesPerChunk(
			ChunkResolution chunkResolution
		) noexcept
		{
			return 3ULL * chunkResolution * ((chunkResolution + 1ULL) * (chunkResolution + 1ULL));
		}

		[[nodiscard]] static constexpr u64 getIndicesPerChunk(
			ChunkResolution chunkResolution
		) noexcept
		{
			return 15ULL * chunkResolution * chunkResolution * chunkResolution;
		}


		[[nodiscard]] static std::size_t ComputeChunkCount(
			const Camera* camera,
			ChunkSize chunkSize
		)
		{
			return getCandidates(camera, chunkSize, kAllocatorShellLayers).size();
		}

		[[nodiscard]] static std::vector<ChunkId> addShell(
			const std::vector<ChunkId>& base,
			i64 layers = 0
		)
		{
			if (layers <= 0) {
				return base;
			}
			std::unordered_set<ChunkId> inflated{ base.begin(), base.end() };
			for (const auto& c : base) {
				for (i64 dx = -layers; dx <= layers; ++dx) {
					for (i64 dy = -layers; dy <= layers; ++dy) {
						for (i64 dz = -layers; dz <= layers; ++dz) {
							if (dx == 0 && dy == 0 && dz == 0) {
								continue;
							}
							inflated.insert(c + glm::i64vec3{ dx, dy, dz });
						}
					}
				}
			}
			std::vector<ChunkId> result{ inflated.begin(), inflated.end() };
			std::sort(result.begin(), result.end(), ChunkIdLess{});
			return result;
		}

		[[nodiscard]] ChunkMapType computeChunkMap() const
		{
			auto candidates = getCandidates(m_Camera, m_ChunkSize, kAllocatorShellLayers);
			ChunkMapType result{};
			result.reserve(candidates.size());
			std::size_t idx = 0;
			for (const auto& chunkCoord : candidates) {
				result.try_emplace(chunkCoord, idx++);
			}
			return result;
		}

		[[nodiscard]] static std::vector<ChunkId> getCandidates(
			const Camera* camera,
			ChunkSize chunkSize,
			i64 shellLayers = 0
		)
		{
			const auto& aabb = camera->getFrustumAABB();
			const auto& frustum = camera->getFrustum();

			// careful, these are float from camera; promote to double *first*
			const glm::dvec3 minW = glm::dvec3{ aabb.min };
			const glm::dvec3 maxW = glm::dvec3{ aabb.max };

			//// Enforce half-open [min, max): push min up by 1 ULP, max down by 1 ULP.
			// min/max in world units promoted to double already
			const ChunkId minChunk = bumped_floor_div<i64>(minW, static_cast<f64>(chunkSize));
			const ChunkId maxChunk = bumped_ceil_div<i64>(maxW, static_cast<f64>(chunkSize));

			std::vector<ChunkId> candidates{};
			candidates.reserve(
				(maxChunk.x - minChunk.x) *
				(maxChunk.y - minChunk.y) *
				(maxChunk.z - minChunk.z)
			);

			for (i64 x = minChunk.x; x < maxChunk.x; ++x) {
				for (i64 y = minChunk.y; y < maxChunk.y; ++y) {
					for (i64 z = minChunk.z; z < maxChunk.z; ++z) {
						const ChunkId base = ChunkId{ x, y, z } *static_cast<i64>(chunkSize);
						const AABB box{ base, base + static_cast<i64>(chunkSize) };
						if (frustum.intersects(box, 0.0f * glm::length(box.max - box.min))) {
							candidates.emplace_back(x, y, z);
						}
					}
				}
			}

			return addShell(candidates, shellLayers);
		}

		void updateDirtyChunks()
		{
			m_DirtyChunks.clear();

			const auto cameraPos = m_Camera->getPosition();
			const auto cameraInvRot = glm::conjugate(m_Camera->getTransform().getRotation());

			for (const auto& cid : m_VisibleChunks) {
				if (const auto info = getChunkInfo(cid); info == nullptr) {

					const auto chunkId = worldToAllocator(
						cid,
						m_ChunkSize,
						cameraPos,
						cameraInvRot,
						m_BasePos,
						m_BaseRot
					);

					if (!m_ChunkMap.contains(chunkId)) {
						std::println(
							"[ChunkManager] Missing allocator key for world {} {} {} -> alloc {} {} {}",
							cid.x, cid.y, cid.z, chunkId.x, chunkId.y, chunkId.z
						);
						// Optional: dump a couple of nearby keys to see how far off we are
						dumpMapAABB();
						continue; // avoid crashing inside the allocator
					}

					const auto allocationInfo = m_ChunkAllocator.allocate(chunkId);

					m_ChunkCache.put(
						cid,
						ChunkInfo{
							allocationInfo.vertexAllocation.offset,
							0,
							allocationInfo.indexAllocation.offset,
							0,
							ChunkState::Dirty,
						}
					);

					m_DirtyChunks.push_back(cid);
				}
			}
		}

		[[nodiscard]] ChunkId worldToAllocator(
			const ChunkId& worldChunk,
			ChunkSize chunkSize,
			const glm::vec3 currentPos,
			const glm::quat& invCurrentRot,
			const glm::vec3& basePos,
			const glm::quat& baseRot
		) const noexcept
		{
			const f32 cs = static_cast<f32>(chunkSize);

			// chunk center in world
			const glm::vec3 wmin = glm::vec3{ worldChunk } *cs;
			const glm::vec3 wcent = wmin + glm::vec3{ cs * 0.5f };

			// undo current pose, apply base pose
			const glm::vec3 localC = (baseRot * (invCurrentRot * (wcent - currentPos))) + basePos;

			// half-open cells: [k*cs,(k+1)*cs).
			return floor_div<i64>(localC, cs);
		}

		void dumpMapAABB() const
		{
			if (m_ChunkMap.empty()) {
				std::println("[ChunkManager] m_ChunkMap is EMPTY");
				return;
			}
			glm::i64vec3 mn{ std::numeric_limits<i64>::max() };
			glm::i64vec3 mx{ std::numeric_limits<i64>::min() };
			for (auto& [k, _] : m_ChunkMap) {
				mn = glm::min(mn, k);
				mx = glm::max(mx, k);
			}
			std::println("[ChunkManager] map AABB: min=({}, {}, {}) max=({}, {}, {}) (count={})",
				mn.x, mn.y, mn.z, mx.x, mx.y, mx.z, m_ChunkMap.size());
		}

		const Camera* m_Camera{};
		glm::vec3 m_BasePos{};
		glm::quat m_BaseRot{};
		ChunkResolution m_ChunkResolution{};
		VoxelSize m_VoxelSize{};
		ChunkSize m_ChunkSize{};
		ChunkMapType m_ChunkMap{};
		ChunkCache m_ChunkCache{};
		std::vector<ChunkId> m_VisibleChunks{};
		std::vector<ChunkId> m_DirtyChunks{};
		ChunkAllocatorType m_ChunkAllocator{};
	};
}

#endif // !MARCHING_CUBES_MANAGERS_CHUNK_MANAGER_HPP

