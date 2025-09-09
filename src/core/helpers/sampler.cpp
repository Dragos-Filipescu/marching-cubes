#include <core/helpers/sampler.hpp>

#include <stdexcept>

namespace marching_cubes::core::helpers {

    VkSampler createSampler(
        VkDevice device,
        const VkSamplerCreateInfo& createInfo,
        const VkAllocationCallbacks* pAllocator
    ) {
        VkSampler sampler{};
        if (vkCreateSampler(
            device,
            &createInfo,
            pAllocator,
            &sampler
        ) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to create sampler!" };
        }
        return sampler;
    }

}
