#pragma once
#ifndef MARCHING_CUBES_CORE_DEVICE_MEMORY_HPP
#define MARCHING_CUBES_CORE_DEVICE_MEMORY_HPP

#include <vulkan/vulkan_core.h>

#include <core/wrapper.hpp>

namespace marching_cubes::core {

	class DeviceMemory final : public WrapperTraits<DeviceMemory, VkDeviceMemory> {
	public:

		DeviceMemory() noexcept = default;

		DeviceMemory(
			VkDevice device,
			VkPhysicalDevice physicalDevice,
			VkMemoryRequirements memoryRequirements,
			VkMemoryPropertyFlags properties
		);

		DeviceMemory(const DeviceMemory&) = delete;
		DeviceMemory& operator=(const DeviceMemory&) = delete;

		DeviceMemory(DeviceMemory&&) noexcept = default;
		DeviceMemory& operator=(DeviceMemory&&) noexcept = default;

		~DeviceMemory() = default;

		[[nodiscard]] VkMemoryPropertyFlags getProperties() const noexcept;
		[[nodiscard]] VkDeviceMemory getWrapped() const noexcept;

	private:
		// owned
		VkMemoryPropertyFlags			m_Properties{};
		OwningWrapper<VkDeviceMemory>	m_Memory{};
	};
}

#endif // !MARCHING_CUBES_CORE_DEVICE_MEMORY_HPP

