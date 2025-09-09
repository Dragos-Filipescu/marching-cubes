#include <core/helpers/device_memory.hpp>

#include <vulkan/vulkan_core.h>

#include <stdexcept>

#include  <core/aliases.hpp>

namespace marching_cubes::core::helpers {

    u32 findMemoryType(
        VkPhysicalDevice physicalDeivce,
        u32 typeFilter,
        VkMemoryPropertyFlags properties
    ) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDeivce, &memProperties);
        for (u32 i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error{ "Failed to find suitable memory type!" };
    }

    VkDeviceMemory allocateDeviceMemory(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkMemoryRequirements memoryRequirements,
        VkMemoryPropertyFlags properties,
        const void* pNext,
        const VkAllocationCallbacks* pAllocator
    ) {
        if (memoryRequirements.size == 0) {
            return VK_NULL_HANDLE;
        }

        VkMemoryAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = pNext,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = findMemoryType(
                physicalDevice,
                memoryRequirements.memoryTypeBits,
                properties
            )
        };

        VkDeviceMemory deviceMemory{};
        if (vkAllocateMemory(
            device,
            &allocInfo,
            pAllocator,
            &deviceMemory
        ) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to allocate buffer memory!" };
        }
        return deviceMemory;
    }

}
