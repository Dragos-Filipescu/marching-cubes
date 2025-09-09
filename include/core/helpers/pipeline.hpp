#pragma once
#ifndef MARCHING_CUBES_CORE_HELPERS_PIPELINE_HPP
#define MARCHING_CUBES_CORE_HELPERS_PIPELINE_HPP

#include <vulkan/vulkan_core.h>

#include <vector>

namespace marching_cubes::core::helpers {

	[[nodiscard]] VkPipelineLayout createPipelineLayout(
		VkDevice device,
		const VkPipelineLayoutCreateInfo& createInfo,
		const VkAllocationCallbacks* pAllocator = nullptr
	);

	[[nodiscard]] std::vector<VkPipeline> createGraphicsPipelines(
		VkDevice device,
		const std::vector<VkGraphicsPipelineCreateInfo>& createInfos,
		VkPipelineCache pipelineCache = VK_NULL_HANDLE,
		const VkAllocationCallbacks* pAllocator = nullptr
	);

	[[nodiscard]] std::vector<VkPipeline> createComputePipelines(
		VkDevice device,
		const std::vector<VkComputePipelineCreateInfo>& createInfos,
		VkPipelineCache pipelineCache = VK_NULL_HANDLE,
		const VkAllocationCallbacks* pAllocator = nullptr
	);
}

#endif // !MARCHING_CUBES_CORE_HELPERS_PIPELINE_HPP

