#include <core/helpers/command_pool.hpp>

#include <vulkan/vulkan_core.h>

#include <stdexcept>

namespace marching_cubes::core::helpers {

    VkCommandPool createCommandPool(
        VkDevice device,
        const VkCommandPoolCreateInfo& createInfo,
        const VkAllocationCallbacks* pAllocator
    ) {
        VkCommandPool commandPool{};
        if (vkCreateCommandPool(
            device,
            &createInfo,
            pAllocator,
            &commandPool
        ) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to create command pool!" };
        }
        return commandPool;
    }
}
