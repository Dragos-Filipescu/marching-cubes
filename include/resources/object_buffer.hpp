#pragma once
#ifndef MARCHING_CUBES_RESOURCES_OBJECT_BUFFER_HPP
#define MARCHING_CUBES_RESOURCES_OBJECT_BUFFER_HPP

#include <vulkan/vulkan_core.h>

#include <cstring>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <core/buffer.hpp>
#include <core/device_memory.hpp>
#include <utils/utils.hpp>

namespace marching_cubes::resources {

	using core::Buffer;
	using core::DeviceMemory;

	template<typename T>
	concept BufferElement =
		std::is_trivially_copyable_v<T>
		&& std::is_standard_layout_v<T>;

	template<BufferElement T>
	class StagedObjectBuffer final {
	public:

		using ElementType = T;

		static constexpr std::size_t kElementSize = sizeof(T);

		StagedObjectBuffer() noexcept = default;

		StagedObjectBuffer(
			VkDevice device,
			VkPhysicalDevice physicalDevice,
			const VkBufferCreateInfo& createInfo
		)
			: m_Device{ device },
			m_PhysicalDevice{ physicalDevice },
			m_Buffer{
				device,
				createInfo
			},
			m_Memory{
				device,
				physicalDevice,
				m_Buffer.getMemoryRequirements(),
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			}
		{
			if (m_Memory != VK_NULL_HANDLE) {
				bindBufferMemory(device, m_Buffer, m_Memory);
			}
		}


		StagedObjectBuffer(const StagedObjectBuffer&) = delete;
		StagedObjectBuffer& operator=(const StagedObjectBuffer&) = delete;

		StagedObjectBuffer(StagedObjectBuffer&&) noexcept = default;
		StagedObjectBuffer& operator=(StagedObjectBuffer&&) noexcept = default;

		~StagedObjectBuffer() = default;

		[[nodiscard]] constexpr VkDevice getDevice() const noexcept
		{
			return m_Device;
		}
		[[nodiscard]] constexpr VkPhysicalDevice getPhysicalDevice() const noexcept
		{
			return m_PhysicalDevice;
		}
		[[nodiscard]] constexpr VkDeviceSize getSize() const noexcept
		{
			return m_Buffer.getCreateInfo().size;
		}
		[[nodiscard]] constexpr VkDeviceSize getElementCount() const noexcept
		{
			return m_Buffer.getCreateInfo().size / kElementSize;
		}
		[[nodiscard]] constexpr const Buffer& getBuffer() const noexcept
		{
			return m_Buffer;
		}
		[[nodiscard]] constexpr Buffer& getBuffer() noexcept
		{
			return m_Buffer;
		}
		[[nodiscard]] constexpr const DeviceMemory& getMemory() const noexcept
		{
			return m_Memory;
		}

		StagedObjectBuffer& updateSize(
			VkDeviceSize newSize
		)
		{
			const auto alignedSize = m_Buffer.alignUp(newSize);
			if (m_Buffer.getCreateInfo().size == 0 || alignedSize > m_Buffer.getAllocationSize()) {

				const auto bufferUsage = m_Buffer.getCreateInfo().usage;
				const auto bufferSharingMode = m_Buffer.getCreateInfo().sharingMode;
				const auto memoryProperties = m_Memory.getProperties();

				m_Buffer = Buffer{};
				m_Memory = DeviceMemory{};

				m_Buffer = Buffer{
					m_Device,
					VkBufferCreateInfo{
						.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
						.pNext = nullptr,
						.size = newSize,
						.usage = bufferUsage,
						.sharingMode = bufferSharingMode,
					}
				};
				m_Memory = DeviceMemory{
					m_Device,
					m_PhysicalDevice,
					m_Buffer.getMemoryRequirements(),
					memoryProperties,
				};

				if (m_Memory != VK_NULL_HANDLE) {
					bindBufferMemory(m_Device, m_Buffer, m_Memory);
				}
			}
			return *this;
		}

	private:
		// non-owned
		VkDevice			m_Device{};
		VkPhysicalDevice	m_PhysicalDevice{};
		// owned
		Buffer				m_Buffer{};
		DeviceMemory		m_Memory{};
	};

	template<BufferElement T>
	class DirectObjectBuffer final {
	public:

		using ElementType = T;
		using PointerType = std::add_pointer_t<ElementType>;
		using ConstPointerType = std::add_pointer_t<std::add_const_t<ElementType>>;

		static constexpr std::size_t kElementSize = sizeof(ElementType);

		DirectObjectBuffer() = default;

		DirectObjectBuffer(
			VkDevice device,
			VkPhysicalDevice physicalDevice,
			const std::vector<T>& elements,
			const VkBufferCreateInfo& createInfo
		)
			: m_Device{ device },
			m_PhysicalDevice{ physicalDevice },
			m_Buffer{
				device,
				createInfo,
			},
			m_Memory{
				device,
				physicalDevice,
				m_Buffer.getMemoryRequirements(),
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			},
			m_Data{ nullptr }
		{
			if (m_Memory != VK_NULL_HANDLE) {
				bindBufferMemory(device, m_Buffer, m_Memory);
				m_Data = static_cast<PointerType>(
					copyDataToDevice(
						device,
						m_Memory,
						elements.data(),
						m_Buffer.getCreateInfo().size,
						false
					)
				);
			}
		}

		DirectObjectBuffer(const DirectObjectBuffer&) = delete;
		DirectObjectBuffer& operator=(const DirectObjectBuffer&) = delete;

		DirectObjectBuffer(DirectObjectBuffer&&) noexcept = default;
		DirectObjectBuffer& operator=(DirectObjectBuffer&&) noexcept = default;

		~DirectObjectBuffer() = default;

		[[nodiscard]] constexpr VkDevice getDevice() const noexcept
		{
			return m_Device;
		}
		[[nodiscard]] constexpr VkPhysicalDevice getPhysicalDevice() const noexcept
		{
			return m_PhysicalDevice;
		}
		[[nodiscard]] constexpr VkDeviceSize getSize() const noexcept
		{
			return m_Buffer.getCreateInfo().size;
		}
		[[nodiscard]] constexpr VkDeviceSize getElementCount() const noexcept
		{
			return m_Buffer.getCreateInfo().size / kElementSize;
		}
		[[nodiscard]] constexpr const Buffer& getBuffer() const noexcept
		{
			return m_Buffer;
		}
		[[nodiscard]] constexpr const DeviceMemory& getMemory() const noexcept
		{
			return m_Memory;
		}
		[[nodiscard]] constexpr ConstPointerType getData() const noexcept
		{
			return m_Data;
		}

		DirectObjectBuffer& updateData(
			std::span<const T> data,
			VkDeviceSize element_offset
		)
		{
			const auto size = data.size_bytes();
			if (m_Buffer.alignUp(size) > m_Buffer.getAllocationSize()) {
				throw std::out_of_range{ "Attempt to write beyond buffer bounds." };
			}
			std::memcpy(
				static_cast<void*>(m_Data + element_offset),
				static_cast<const void*>(std::as_bytes(data).data()),
				size
			);
			return *this;
		}

	private:
		// non-owned
		VkDevice			m_Device{};
		VkPhysicalDevice	m_PhysicalDevice{};
		// owned
		Buffer				m_Buffer{};
		DeviceMemory		m_Memory{};
		PointerType			m_Data{};
	};
}
#endif // !MARCHING_CUBES_RESOURCES_OBJECT_BUFFER_HPP
