#pragma once
#ifndef MARCHING_CUBES_CORE_HELPERS_SEMAPHORE_HPP
#define MARCHING_CUBES_CORE_HELPERS_SEMAPHORE_HPP

#include <vulkan/vulkan_core.h>

namespace marching_cubes::core::helpers {

	[[nodiscard]] VkSemaphore createSemaphore(
		VkDevice device,
		const VkSemaphoreCreateInfo& createInfo,
		const VkAllocationCallbacks* pAllocator = nullptr
	);
}

#endif // !MARCHING_CUBES_CORE_HELPERS_SEMAPHORE_HPP

