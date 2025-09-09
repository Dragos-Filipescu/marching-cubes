#include <core/helpers/semaphore.hpp>

#include <stdexcept>

namespace marching_cubes::core::helpers {

    VkSemaphore createSemaphore(
        VkDevice device,
        const VkSemaphoreCreateInfo& createInfo,
        const VkAllocationCallbacks* pAllocator
    ) {
        VkSemaphore semaphore{};
        if (vkCreateSemaphore(
            device,
            &createInfo,
            pAllocator,
            &semaphore
        ) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to create semaphore!" };
        }
        return semaphore;
    }
}
