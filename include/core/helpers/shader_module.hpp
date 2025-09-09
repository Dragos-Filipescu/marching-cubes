#pragma once
#ifndef MARCHING_CUBES_CORE_HELPERS_SHADER_MODULE_HPP
#define MARCHING_CUBES_CORE_HELPERS_SHADER_MODULE_HPP

#include <vulkan/vulkan_core.h>

#include <vector>

namespace marching_cubes::core::helpers {

	[[nodiscard]] VkShaderModule createShaderModule(
		VkDevice device,
		const std::vector<char>& code,
		const VkAllocationCallbacks* pAllocator = nullptr
	);
}

#endif // !MARCHING_CUBES_CORE_HELPERS_SHADER_MODULE_HPP

