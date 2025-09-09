#include <core/helpers/shader_module.hpp>

#include <cstdint>
#include <stdexcept>

namespace marching_cubes::core::helpers {

    VkShaderModule createShaderModule(
        VkDevice device,
        const std::vector<char>& code,
        const VkAllocationCallbacks* pAllocator
    ) {
        VkShaderModuleCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = code.size(),
            .pCode = reinterpret_cast<const std::uint32_t*>(code.data()),
        };

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(
            device,
            &createInfo,
            pAllocator,
            &shaderModule
        ) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to create shader module!" };
        }

        return shaderModule;
    }
}
