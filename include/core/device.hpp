#pragma once
#ifndef MARCHING_CUBES_CORE_DEVICE_HPP
#define MARCHING_CUBES_CORE_DEVICE_HPP

#include <vulkan/vulkan_core.h>

#include <core/wrapper.hpp>

namespace marching_cubes::core {

	class Device final : public WrapperTraits<Device, VkDevice> {
	public:

		Device() noexcept = default;

		Device(
			VkDevice device,
			VkQueue graphicsQueue,
			VkQueue presentQueue
		) noexcept;

		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;

		Device(Device&&) noexcept = default;
		Device& operator=(Device&&) noexcept = default;

		~Device() = default;

		[[nodiscard]] VkDevice getWrapped() const noexcept;
		[[nodiscard]] VkQueue getGraphicsQueue() const noexcept;
		[[nodiscard]] VkQueue getPresentQueue() const noexcept;

	private:
		// owned
		OwningWrapper<VkDevice> m_Device{};
		// owned implicitly by device
		VkQueue					m_GraphicsQueue{};
		VkQueue					m_PresentQueue{};
	};
}
#endif // !MARCHING_CUBES_CORE_DEVICE_HPP
