#include <core/helpers/framebuffer.hpp>

#include <vulkan/vulkan_core.h>

#include <span>
#include <stdexcept>

#include <core/aliases.hpp>

namespace marching_cubes::core::helpers {

    VkFramebuffer createFrameBuffer(
        VkDevice device,
        const VkFramebufferCreateInfo& createInfo,
        const VkAllocationCallbacks* pAllocator
    ) {
        VkFramebuffer frameBuffer{};
        if (vkCreateFramebuffer(
            device,
            &createInfo,
            pAllocator,
            &frameBuffer
        ) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to create framebuffer!" };
        }
        return frameBuffer;
    }

    VkFramebufferCreateInfo makeFramebufferCreateInfo(
        VkRenderPass renderPass,
        const std::span<VkImageView>& attachments,
        VkExtent2D extent
    ) {
        return VkFramebufferCreateInfo{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderPass = renderPass,
            .attachmentCount = static_cast<u32>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = extent.width,
            .height = extent.height,
            .layers = 1
        };
    }

}
