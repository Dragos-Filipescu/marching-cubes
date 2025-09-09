#include <core/helpers/image.hpp>

#include <stdexcept>

namespace marching_cubes::core::helpers {

    VkImage createImage(
        VkDevice device,
        const VkImageCreateInfo& createInfo,
        const VkAllocationCallbacks* pAllocator
    ) {
        VkImage image{};
        if (vkCreateImage(
            device,
            &createInfo,
            pAllocator,
            &image
        ) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to create image!" };
        }
        return image;
    }

    VkMemoryRequirements getImageMemoryRequirements(
        VkDevice device,
        VkImage image
    ) {
        VkMemoryRequirements memoryRequirements;
        vkGetImageMemoryRequirements(device, image, &memoryRequirements);
        return memoryRequirements;
    }

}
