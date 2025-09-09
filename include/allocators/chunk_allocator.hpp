#pragma once
#ifndef MARCHING_CUBES_ALLOCATORS_CHUNK_ALLOCATOR_HPP
#define MARCHING_CUBES_ALLOCATORS_CHUNK_ALLOCATOR_HPP

#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

#include <concepts>
#include <cstdint>
#include <utility>

#include <core/aliases.hpp>
#include <allocators/slab_allocator.hpp>
#include <resources/object_buffer.hpp>
#include <scene/vertex.hpp>
#include <utils/utils.hpp>

namespace marching_cubes::allocators {

	using scene::Vertex;
	using resources::StagedObjectBuffer;

	template<
		Vertex VertexT,
		BufferLike VertexAllocatorBufferT,
		std::unsigned_integral IndexT,
		BufferLike IndexAllocatorBufferT,
		typename CoordT,
		typename IndexerT
	>
		requires (
			std::default_initializable<VertexAllocatorBufferT>
			&& std::default_initializable<IndexAllocatorBufferT>
			&& std::move_constructible<VertexAllocatorBufferT>
			&& std::move_constructible<IndexAllocatorBufferT>
			&& Indexer<IndexerT, CoordT>
		)
	class BasicChunkAllocator final {

	public:

		using VertexType = VertexT;
		using IndexType = IndexT;
		using CoordType = CoordT;
		using IndexerType = IndexerT;

		using VertexAllocatorType = BasicSlabAllocator<
			VertexT,
			VertexAllocatorBufferT,
			CoordT,
			IndexerT
		>;

		using IndexAllocatorType = BasicSlabAllocator<
			IndexT,
			IndexAllocatorBufferT,
			CoordT,
			IndexerT
		>;

		struct AllocationInfo final {
			VertexAllocatorType::Allocation vertexAllocation;
			IndexAllocatorType::Allocation indexAllocation;
		};

		static constexpr u64 c_MaxTrianglesPerChunk = 5ULL;
		static constexpr u64 c_MaxVerticesPerChunk = c_MaxTrianglesPerChunk * 3ULL;
		static constexpr u64 c_MaxIndicesPerChunk = c_MaxTrianglesPerChunk * 3ULL;

		BasicChunkAllocator() noexcept = default;

		template<
			typename VertexAllocatorBufferT,
			typename IndexAllocatorBufferT,
			typename VertexIndexerT,
			typename IndexIndexerT
		>
		BasicChunkAllocator(
			u64 verticesPerChunk,
			VertexAllocatorBufferT&& vertexBuffer,
			VertexIndexerT&& vertexBufferIndexer,
			u64 indicesPerChunk,
			IndexAllocatorBufferT&& indexBuffer,
			IndexIndexerT&& indexBufferIndexer
		) noexcept
			: m_VertexAllocator{
				verticesPerChunk,
				std::forward<VertexAllocatorBufferT>(vertexBuffer),
				std::forward<IndexerT>(vertexBufferIndexer),
			},
			m_IndexAllocator{
				indicesPerChunk,
				std::forward<IndexAllocatorBufferT>(indexBuffer),
				std::forward<IndexerT>(indexBufferIndexer),
			}
		{
		}

		BasicChunkAllocator(const BasicChunkAllocator&) = delete;
		BasicChunkAllocator& operator=(const BasicChunkAllocator&) = delete;

		BasicChunkAllocator(BasicChunkAllocator&&) noexcept = default;
		BasicChunkAllocator& operator=(BasicChunkAllocator&&) noexcept = default;

		~BasicChunkAllocator() noexcept = default;

		[[nodiscard]] AllocationInfo allocate(const CoordType& coords) const
		{
			return AllocationInfo{
				m_VertexAllocator.allocate(coords),
				m_IndexAllocator.allocate(coords),
			};
		}

		template<
			Indexer<CoordT> VertexIndexerT,
			Indexer<CoordT> IndexIndexerT
		>
		void rebindIndexers(
			VertexIndexerT&& vertexIndexer,
			IndexIndexerT&& indexIndexer
		)
		{
			m_VertexAllocator.rebindIndexer(std::forward<VertexIndexerT>(vertexIndexer));
			m_IndexAllocator.rebindIndexer(std::forward<IndexIndexerT>(indexIndexer));
		}

		[[nodiscard]] const auto& getVertexBuffer() const noexcept
		{
			return m_VertexAllocator.getBuffer();
		}

		[[nodiscard]] auto& getVertexBuffer() noexcept
		{
			return m_VertexAllocator.getBuffer();
		}

		[[nodiscard]] const auto& getIndexBuffer() const noexcept
		{
			return m_IndexAllocator.getBuffer();
		}

		[[nodiscard]] auto& getIndexBuffer() noexcept
		{
			return m_IndexAllocator.getBuffer();
		}

	private:

		VertexAllocatorType m_VertexAllocator{};
		IndexAllocatorType m_IndexAllocator{};
	};
}

#endif // !MARCHING_CUBES_ALLOCATORS_CHUNK_ALLOCATOR_HPP

