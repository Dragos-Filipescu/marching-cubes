#pragma once
#ifndef MARCHING_CUBES_CORE_HELPERS_SAMPLER_HPP
#define MARCHING_CUBES_CORE_HELPERS_SAMPLER_HPP

#include <vulkan/vulkan_core.h>

namespace marching_cubes::core::helpers {

	[[nodiscard]] VkSampler createSampler(
		VkDevice device,
		const VkSamplerCreateInfo& createInfo,
		const VkAllocationCallbacks* pAllocator = nullptr
	);
}

#endif // !MARCHING_CUBES_CORE_HELPERS_SAMPLER_HPP

