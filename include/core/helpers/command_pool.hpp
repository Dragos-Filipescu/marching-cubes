#pragma once
#ifndef MARCHING_CUBES_CORE_HELPERS_COMMAND_POOL_HPP
#define MARCHING_CUBES_CORE_HELPERS_COMMAND_POOL_HPP

#include <vulkan/vulkan_core.h>

namespace marching_cubes::core::helpers {

    [[nodiscard]] VkCommandPool createCommandPool(
        VkDevice device,
        const VkCommandPoolCreateInfo& createInfo,
        const VkAllocationCallbacks* pAllocator = nullptr
    );
}

#endif // !MARCHING_CUBES_CORE_HELPERS_COMMAND_POOL_HPP

