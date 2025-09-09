#pragma once
#ifndef MARCHING_CUBES_CORE_HELPERS_SWAPCHAIN_HPP
#define MARCHING_CUBES_CORE_HELPERS_SWAPCHAIN_HPP

#include <vulkan/vulkan_core.h>

#include <vector>

#include <core/wrapper.hpp>

namespace marching_cubes::core::helpers {

	[[nodiscard]] std::vector<VkImage> getSwapChainImages(
		VkDevice device,
		VkSwapchainKHR swapchain
	);

	[[nodiscard]] std::vector<OwningWrapper<VkImageView>> createImageViews(
		VkDevice device,
		const std::vector<VkImage>& swapchainImages,
		VkFormat format
	);

	[[nodiscard]] std::vector<OwningWrapper<VkSemaphore>> createRenderFinishedSemaphores(
		VkDevice device,
		std::size_t count
	);
}

#endif // !MARCHING_CUBES_CORE_HELPERS_SWAPCHAIN_HPP

