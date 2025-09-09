#include <core/helpers/fence.hpp>

#include <vulkan/vulkan_core.h>

#include <stdexcept>

namespace marching_cubes::core::helpers {

    VkFence createFence(
        VkDevice device,
        const VkFenceCreateInfo& createInfo,
        const VkAllocationCallbacks* pAllocator
    ) {
        VkFence fence{};
        if (vkCreateFence(
            device,
            &createInfo,
            pAllocator,
            &fence
        ) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to create fence!" };
        }
        return fence;
    }

}
