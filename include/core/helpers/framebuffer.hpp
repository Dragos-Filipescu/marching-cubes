#pragma once
#ifndef MARCHING_CUBES_CORE_HELPERS_FRAMEBUFFER_HPP
#define MARCHING_CUBES_CORE_HELPERS_FRAMEBUFFER_HPP

#include <vulkan/vulkan_core.h>

#include <span>

namespace marching_cubes::core::helpers {

	[[nodiscard]] VkFramebuffer createFrameBuffer(
		VkDevice device,
		const VkFramebufferCreateInfo& createInfo,
		const VkAllocationCallbacks* pAllocator = nullptr
	);

	[[nodiscard]] VkFramebufferCreateInfo makeFramebufferCreateInfo(
		VkRenderPass renderPass,
		const std::span<VkImageView>& attachments,
		VkExtent2D extent
	);
}

#endif // !MARCHING_CUBES_CORE_HELPERS_FRAMEBUFFER_HPP

