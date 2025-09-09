#pragma once
#ifndef MARCHING_CUBES_CORE_HELPERS_FENCE_HPP
#define MARCHING_CUBES_CORE_HELPERS_FENCE_HPP

#include <vulkan/vulkan_core.h>

namespace marching_cubes::core::helpers {

    [[nodiscard]] VkFence createFence(
        VkDevice device,
        const VkFenceCreateInfo& createInfo,
        const VkAllocationCallbacks* pAllocator = nullptr
    );
}

#endif // !MARCHING_CUBES_CORE_HELPERS_FENCE_HPP

