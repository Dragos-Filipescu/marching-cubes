#pragma once
#ifndef MARCHING_CUBES_RESOURCES_IMAGE_RESOURCE_HPP
#define MARCHING_CUBES_RESOURCES_IMAGE_RESOURCE_HPP

#include <vulkan/vulkan_core.h>

#include <core/aliases.hpp>
#include <core/device_memory.hpp>
#include <core/image.hpp>

namespace marching_cubes::resources {

	class ImageResource final {
	public:

		enum class LinearBlitCheck : bool {
			Disable = false,
			Enable = true,
		};

		ImageResource() noexcept = default;

		ImageResource(
			VkDevice device,
			VkPhysicalDevice physicalDevice,
			const VkImageCreateInfo& imageCreateInfo,
			VkMemoryPropertyFlags memoryProperties,
			LinearBlitCheck checkLinearBlitting
		);

		ImageResource(const ImageResource&) = delete;
		ImageResource& operator=(const ImageResource&) = delete;

		ImageResource(ImageResource&& other) = default;
		ImageResource& operator=(ImageResource&& other) = default;

		~ImageResource() = default;

		[[nodiscard]] const core::Image& getImage() const noexcept;
		[[nodiscard]] core::Image& getImage() noexcept;
		[[nodiscard]] const core::DeviceMemory& getMemory() const noexcept;
		[[nodiscard]] core::DeviceMemory& getMemory() noexcept;

		[[nodiscard]] inline auto getCallbackUpdateImageLayout(
			u32 mipLevel,
			VkImageLayout layout
		)
		{
			return [this, mipLevel, layout]() {
				callbackUpdateImageLayout(this, mipLevel, layout);
			};
		}

		[[nodiscard]] inline auto getCallbackUpdateImageLayouts(VkImageLayout layout)
		{
			return [this, layout]() {
				callbackUpdateImageLayouts(this, layout);
			};
		}

	private:

		static void callbackUpdateImageLayout(
			ImageResource* texture,
			u32 mipLevel,
			VkImageLayout newLayout
		);
		static void callbackUpdateImageLayouts(
			ImageResource* texture,
			VkImageLayout newLayout
		);

		core::Image			m_Image{};
		core::DeviceMemory	m_Memory{};
	};
}

#endif // !MARCHING_CUBES_RESOURCES_IMAGE_RESOURCE_HPP

