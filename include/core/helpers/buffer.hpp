#pragma once
#ifndef MARCHING_CUBES_CORE_HELPERS_BUFFER_HPP
#define MARCHING_CUBES_CORE_HELPERS_BUFFER_HPP

#include <vulkan/vulkan_core.h>

namespace marching_cubes::core::helpers {

    [[nodiscard]] VkBuffer createBuffer(
        VkDevice device,
        const VkBufferCreateInfo& createInfo,
        const VkAllocationCallbacks* pAllocator = nullptr
    );

    [[nodiscard]] VkMemoryRequirements getBufferMemoryRequirements(
        VkDevice device,
        VkBuffer buffer
    );
}

#endif // !MARCHING_CUBES_CORE_HELPERS_BUFFER_HPP

