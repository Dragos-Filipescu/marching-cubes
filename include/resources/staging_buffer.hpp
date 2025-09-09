#pragma once
#ifndef MARCHING_CUBES_RESOURCES_STAGING_BUFFER_HPP
#define MARCHING_CUBES_RESOURCES_STAGING_BUFFER_HPP

#include <vulkan/vulkan_core.h>

#include <concepts>
#include <ranges>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <core/buffer.hpp>
#include <core/device_memory.hpp>
#include <utils/utils.hpp>

namespace marching_cubes::resources {

	template<typename T>
		requires std::is_trivially_copyable_v<T>
	&& std::is_standard_layout_v<T>
	class StagingBuffer final {
	public:

		using ElementType = T;

		static inline constexpr std::size_t kElementSize = sizeof(T);

		StagingBuffer(
			VkDevice device,
			VkPhysicalDevice physicalDevice,
			const std::span<const T>& data
		)
			: m_Device{ device },
			m_PhysicalDevice{ physicalDevice },
			m_Buffer{
				device,
				VkBufferCreateInfo{
					.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.size = data.size() * kElementSize,
					.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					.sharingMode = VK_SHARING_MODE_EXCLUSIVE
				}
			},
			m_Memory{
				device,
				physicalDevice,
				m_Buffer.getMemoryRequirements(),
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			}
		{
			if (m_Memory != VK_NULL_HANDLE) {
				bindBufferMemory(device, m_Buffer, m_Memory);
				copyDataToDevice(
					device,
					m_Memory,
					data.data(),
					m_Buffer.getCreateInfo().size
				);
			}
		}

		StagingBuffer(const StagingBuffer&) = delete;
		StagingBuffer& operator=(const StagingBuffer&) = delete;

		StagingBuffer(StagingBuffer&&) noexcept = default;
		StagingBuffer& operator=(StagingBuffer&&) noexcept = default;

		~StagingBuffer() = default;

		[[nodiscard]] constexpr VkDeviceSize getSize() const noexcept
		{
			return m_Buffer.getCreateInfo().size;
		}

		[[nodiscard]] constexpr VkDeviceSize getElementCount() const noexcept
		{
			return m_Buffer.getCreateInfo().size / kElementSize;
		}

		[[nodiscard]] constexpr const marching_cubes::core::Buffer& getBuffer() const noexcept
		{
			return m_Buffer;
		}

		[[nodiscard]] constexpr marching_cubes::core::Buffer& getBuffer() noexcept
		{
			return m_Buffer;
		}

		[[nodiscard]] constexpr const marching_cubes::core::DeviceMemory& getMemory() const noexcept
		{
			return m_Memory;
		}

		StagingBuffer& updateData(const std::span<const T>& data) {
			const auto newSize = data.size();
			const auto alignedSize = m_Buffer.alignUp(newSize);

			if (alignedSize > m_Buffer.getAllocationSize()) {

				const auto bufferUsage = m_Buffer.getCreateInfo().usage;
				const auto bufferSharingMode = m_Buffer.getCreateInfo().sharingMode;
				const auto memoryProperties = m_Memory.getProperties();

				m_Buffer = marching_cubes::core::Buffer{
					m_Device,
					VkBufferCreateInfo{
						.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
						.pNext = nullptr,
						.size = newSize,
						.usage = m_Buffer.getCreateInfo().usage,
						.sharingMode = m_Buffer.getCreateInfo().sharingMode
					}
				};

				m_Memory = marching_cubes::core::DeviceMemory{
					m_Device,
					m_PhysicalDevice,
					m_Buffer.getMemoryRequirements(),
					m_Memory.getProperties()
				};

				if (m_Memory != VK_NULL_HANDLE) {
					bindBufferMemory(m_Device, m_Buffer, m_Memory);
				}
			}
			copyDataToDevice(
				m_Device,
				m_Memory,
				data.data(),
				newSize
			);
			return *this;
		}

	private:
		// non-owned
		VkDevice							m_Device;
		VkPhysicalDevice					m_PhysicalDevice;
		// owned
		marching_cubes::core::Buffer		m_Buffer;
		marching_cubes::core::DeviceMemory	m_Memory;
	};

	// Deduction guide for containers (vector, array, etc.)
	template<typename Container>
		requires std::ranges::contiguous_range<Container>
	&& std::is_trivially_copyable_v<std::ranges::range_value_t<Container>>
	&& std::is_standard_layout_v<std::ranges::range_value_t<Container>>
	StagingBuffer(
		VkDevice,
		VkPhysicalDevice,
		const Container&
	) -> StagingBuffer<std::ranges::range_value_t<Container>>;

	template<typename T>
		requires std::is_trivially_copyable_v<T>
	&& std::is_standard_layout_v<T>
	StagingBuffer(
		VkDevice,
		VkPhysicalDevice,
		const std::span<T>&
	) -> StagingBuffer<T>;
}

#endif // !MARCHING_CUBES_RESOURCES_STAGING_BUFFER_HPP

