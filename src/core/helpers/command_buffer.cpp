#include <core/helpers/command_buffer.hpp>

#include <vulkan/vulkan_core.h>

#include <limits>
#include <stdexcept>

#include <core/aliases.hpp>

namespace marching_cubes::core::helpers {

    std::vector<VkCommandBuffer> createCommandBuffers(
        VkDevice device,
        const VkCommandBufferAllocateInfo& allocateInfo
    ) {
        std::vector<VkCommandBuffer> commandBuffers(allocateInfo.commandBufferCount);
        if (vkAllocateCommandBuffers(
            device,
            &allocateInfo,
            commandBuffers.data()
        ) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to allocate command buffers!" };
        }

        return commandBuffers;
    }

    VkCommandBuffer beginSingleTimeCommands(
        VkDevice device,
        VkCommandPool commandPool
    ) noexcept {
        VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        VkCommandBuffer commandBuffer{};
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void endSingleTimeCommands(
        VkDevice device,
        VkCommandPool commandPool,
        VkCommandBuffer commandBuffer,
        VkQueue submitQueue,
        const VkFence* waitFence
    ) noexcept {

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer
        };

        vkQueueSubmit(submitQueue, 1, &submitInfo, *waitFence);

        if (*waitFence != VK_NULL_HANDLE) {
            vkWaitForFences(device, 1, waitFence, VK_TRUE, std::numeric_limits<u64>::max());
        }
        else {
            vkQueueWaitIdle(submitQueue);
        }

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }
}
