#pragma once
#ifndef MARCHING_CUBES_ALLOCATORS_SLAB_ALLOCATOR_HPP
#define MARCHING_CUBES_ALLOCATORS_SLAB_ALLOCATOR_HPP

#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

#include <concepts>
#include <type_traits>
#include <utility>

namespace marching_cubes::allocators {

	template<typename T>
	concept BufferLike = requires(T buf, VkDeviceSize newSize) {
		{ buf.getSize() } -> std::convertible_to<std::size_t>;
		{ buf.getBuffer() };
		{ buf.updateSize(newSize) };
	};

	template<typename T, typename CoordT>
	concept Indexer = std::invocable<T, CoordT>
		&& std::convertible_to<
			std::invoke_result_t<T, CoordT>,
			std::size_t
		>;

	template<
		typename ElementT,
		BufferLike BufferT,
		typename CoordT,
		Indexer<CoordT> IndexerT
	>
		requires (
			std::default_initializable<BufferT>
			&& std::default_initializable<IndexerT>
			&& std::move_constructible<BufferT>
			&& std::move_constructible<IndexerT>
		)
	class BasicSlabAllocator final {
	public:

		using ElementType = ElementT;
		using BufferType = BufferT;
		using CoordType = CoordT;
		using IndexerType = IndexerT;

		struct Allocation final {
			VkDeviceSize offset{};
			VkDeviceSize count{};
		};

		BasicSlabAllocator() noexcept = default;

		BasicSlabAllocator(
			std::size_t elementsPerSlot,
			BufferType&& buffer,
			IndexerType&& indexer
		)
			: m_ElementsPerSlot{ elementsPerSlot },
			m_Buffer{ std::forward<BufferType>(buffer) },
			m_Indexer{ std::forward<IndexerType>(indexer) }
		{
		}

		BasicSlabAllocator(const BasicSlabAllocator&) = delete;
		BasicSlabAllocator& operator=(const BasicSlabAllocator&) = delete;

		BasicSlabAllocator(BasicSlabAllocator&&) noexcept = default;
		BasicSlabAllocator& operator=(BasicSlabAllocator&&) noexcept = default;

		~BasicSlabAllocator() noexcept = default;

		[[nodiscard]] Allocation allocate(const CoordType& coords) const
		{
			std::size_t slot = m_Indexer(coords);
			return Allocation{
				.offset = slot * m_ElementsPerSlot * sizeof(ElementType),
				.count = m_ElementsPerSlot,
			};
		}

		template<Indexer<CoordT> IndexerType>
		void rebindIndexer(IndexerType&& indexer)
		{
			m_Indexer = std::forward<IndexerType>(indexer);
		}

		[[nodiscard]] const auto& getBuffer() const noexcept
		{
			return m_Buffer;
		}

		[[nodiscard]] auto& getBuffer() noexcept
		{
			return m_Buffer;
		}

	private:

		std::size_t m_ElementsPerSlot{};
		BufferType m_Buffer{};
		IndexerType m_Indexer{};
	};
}

#endif // !MARCHING_CUBES_ALLOCATORS_SLAB_ALLOCATOR_HPP

