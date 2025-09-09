#include <core/helpers/pipeline_cache.hpp>

#include <stdexcept>

namespace marching_cubes::core::helpers {

	VkPipelineCache createPipelineCache(
		VkDevice device,
		const VkPipelineCacheCreateInfo& createInfo,
		const VkAllocationCallbacks* pAllocator
	) {
		VkPipelineCache pipelineCache{};
		if (vkCreatePipelineCache(
			device,
			&createInfo,
			pAllocator,
			&pipelineCache
		) != VK_SUCCESS) {
			throw std::runtime_error{ "Failed to create pipeline cache!" };
		}
		return pipelineCache;
	}
}
