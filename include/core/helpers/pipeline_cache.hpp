#pragma once
#ifndef MARCHING_CUBES_CORE_HELPERS_PIPELINE_CACHE_HPP
#define MARCHING_CUBES_CORE_HELPERS_PIPELINE_CACHE_HPP

#include <vulkan/vulkan_core.h>

namespace marching_cubes::core::helpers {

	[[nodiscard]] VkPipelineCache createPipelineCache(
		VkDevice device,
		const VkPipelineCacheCreateInfo& createInfo,
		const VkAllocationCallbacks* pAllocator = nullptr
	);
}

#endif // !MARCHING_CUBES_CORE_HELPERS_PIPELINE_CACHE_HPP

