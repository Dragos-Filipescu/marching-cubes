#include <core/helpers/image_view.hpp>

#include <stdexcept>

namespace marching_cubes::core::helpers {

    VkImageView createImageView(
        VkDevice device,
        const VkImageViewCreateInfo& createInfo,
        const VkAllocationCallbacks* pAllocator
    ) {
        VkImageView imageView{};
        if (vkCreateImageView(
            device,
            &createInfo,
            pAllocator,
            &imageView
        ) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to create image view!" };
        }
        return imageView;
    }
}
