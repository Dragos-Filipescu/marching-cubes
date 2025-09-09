#include <core/helpers/pipeline.hpp>

#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace marching_cubes::core::helpers {

	VkPipelineLayout createPipelineLayout(
		VkDevice device,
		const VkPipelineLayoutCreateInfo& createInfo,
		const VkAllocationCallbacks* pAllocator
	) {
		VkPipelineLayout pipelineLayout{};
		if (vkCreatePipelineLayout(
			device,
			&createInfo,
			pAllocator,
			&pipelineLayout
		) != VK_SUCCESS) {
			throw std::runtime_error{ "Failed to create pipeline layout!" };
		}
		return pipelineLayout;
	}

	std::vector<VkPipeline> createGraphicsPipelines(
		VkDevice device,
		const std::vector<VkGraphicsPipelineCreateInfo>& createInfos,
		VkPipelineCache pipelineCache,
		const VkAllocationCallbacks* pAllocator
	) {
		std::size_t count = createInfos.size();
		std::vector<VkPipeline> graphicsPipelines(count);
		if (vkCreateGraphicsPipelines(
			device,
			pipelineCache,
			static_cast<std::uint32_t>(count),
			createInfos.data(),
			pAllocator,
			graphicsPipelines.data()
		) != VK_SUCCESS) {
			throw std::runtime_error{ "Failed to create graphics pipelines!" };
		}
		return graphicsPipelines;
	}

	std::vector<VkPipeline> createComputePipelines(
		VkDevice device,
		const std::vector<VkComputePipelineCreateInfo>& createInfos,
		VkPipelineCache pipelineCache,
		const VkAllocationCallbacks* pAllocator
	) {
		std::size_t count = createInfos.size();
		std::vector<VkPipeline> computePipelines(count);
		if (vkCreateComputePipelines(
			device,
			pipelineCache,
			static_cast<std::uint32_t>(count),
			createInfos.data(),
			pAllocator,
			computePipelines.data()
		) != VK_SUCCESS) {
			throw std::runtime_error{ "Failed to create compute pipelines!" };
		}
		return computePipelines;
	}
}
