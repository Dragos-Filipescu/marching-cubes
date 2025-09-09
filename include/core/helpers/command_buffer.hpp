#pragma once
#ifndef MARCHING_CUBES_CORE_HELPERS_COMMAND_BUFFER_HPP
#define MARCHING_CUBES_CORE_HELPERS_COMMAND_BUFFER_HPP

#include <vulkan/vulkan_core.h>

#include <vector>

namespace marching_cubes::core::helpers {

    [[nodiscard]] std::vector<VkCommandBuffer> createCommandBuffers(
        VkDevice device,
        const VkCommandBufferAllocateInfo& allocateInfo
    );

    [[nodiscard]] VkCommandBuffer beginSingleTimeCommands(
        VkDevice device,
        VkCommandPool commandPool
    ) noexcept;

    void endSingleTimeCommands(
        VkDevice device,
        VkCommandPool commandPool,
        VkCommandBuffer commandBuffer,
        VkQueue submitQueue,
        const VkFence* waitFence
    ) noexcept;

}

#endif // !MARCHING_CUBES_CORE_HELPERS_COMMAND_BUFFER_HPP

