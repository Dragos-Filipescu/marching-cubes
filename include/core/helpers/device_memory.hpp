#pragma once
#ifndef MARCHING_CUBES_CORE_HELPERS_DEVICE_MEMORY_HPP
#define MARCHING_CUBES_CORE_HELPERS_DEVICE_MEMORY_HPP

#include <vulkan/vulkan_core.h>

#include <core/aliases.hpp>

namespace marching_cubes::core::helpers {

    [[nodiscard]] u32 findMemoryType(
        VkPhysicalDevice physicalDeivce,
        u32 typeFilter,
        VkMemoryPropertyFlags properties
    );

    [[nodiscard]] VkDeviceMemory allocateDeviceMemory(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkMemoryRequirements memoryRequirements,
        VkMemoryPropertyFlags properties,
        const void* pNext = nullptr,
        const VkAllocationCallbacks* pAllocator = nullptr
    );
}

#endif // !MARCHING_CUBES_CORE_HELPERS_DEVICE_MEMORY_HPP

