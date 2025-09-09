#pragma once
#ifndef MARCHING_CUBES_CORE_HELPERS_IMAGE_HPP
#define MARCHING_CUBES_CORE_HELPERS_IMAGE_HPP

#include <vulkan/vulkan_core.h>

namespace marching_cubes::core::helpers {

	[[nodiscard]] VkImage createImage(
		VkDevice device,
		const VkImageCreateInfo& createInfo,
		const VkAllocationCallbacks* pAllocator = nullptr
	);

	[[nodiscard]] VkMemoryRequirements getImageMemoryRequirements(
		VkDevice device,
		VkImage image
	);
}

#endif // !MARCHING_CUBES_CORE_HELPERS_IMAGE_HPP

