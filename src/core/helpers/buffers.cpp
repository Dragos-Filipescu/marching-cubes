#include <core/helpers/buffer.hpp>

#include <vulkan/vulkan_core.h>

#include <stdexcept>

namespace marching_cubes::core::helpers {

    VkBuffer createBuffer(
        VkDevice device,
        const VkBufferCreateInfo& createInfo,
        const VkAllocationCallbacks* pAllocator
    ) {
        if (createInfo.size == 0) {
            return VK_NULL_HANDLE;
        }
        VkBuffer buffer{};
        if (vkCreateBuffer(
            device,
            &createInfo,
            pAllocator,
            &buffer
        ) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to create buffer!" };
        }
        return buffer;
    }

    VkMemoryRequirements getBufferMemoryRequirements(
        VkDevice device,
        VkBuffer buffer
    ) {

        VkMemoryRequirements memoryRequirements{};
        if (buffer != VK_NULL_HANDLE) {
            vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);
        }
        return memoryRequirements;
    }
}
