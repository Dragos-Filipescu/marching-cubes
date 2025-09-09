#pragma once
#ifndef MARCHING_CUBES_CORE_HELPERS_IMAGE_VIEW_HPP
#define MARCHING_CUBES_CORE_HELPERS_IMAGE_VIEW_HPP

#include <vulkan/vulkan_core.h>

namespace marching_cubes::core::helpers {

    [[nodiscard]] VkImageView createImageView(
        VkDevice device,
        const VkImageViewCreateInfo& createInfo,
        const VkAllocationCallbacks* pAllocator = nullptr
    );
}

#endif // !MARCHING_CUBES_CORE_HELPERS_IMAGE_VIEW_HPP

