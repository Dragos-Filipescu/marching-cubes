#include "resources/image_resource.hpp"

#include <vulkan/vulkan_core.h>

#include <stdexcept>

#include <core/aliases.hpp>
#include <core/physical_device.hpp>
#include <utils/utils.hpp>

namespace marching_cubes::resources {

    ImageResource::ImageResource(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        const VkImageCreateInfo& imageCreateInfo,
        VkMemoryPropertyFlags memoryProperties,
        ImageResource::LinearBlitCheck checkLinearBlitting
    )
        : m_Image{ device, imageCreateInfo },
        m_Memory{
            device,
            physicalDevice,
            m_Image.getMemoryRequirements(),
            memoryProperties
        }
    {
        if (checkLinearBlitting == ImageResource::LinearBlitCheck::Enable
            && imageCreateInfo.mipLevels > 1
            && !(core::PhysicalDevice::getFormatProperties(
                physicalDevice,
                imageCreateInfo.format
            ).optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)
            ) {
            throw std::runtime_error{ "Texture image format does not support linear blitting!" };
        }

        if (m_Memory != VK_NULL_HANDLE) {
            bindImageMemory(device, m_Image, m_Memory);
        }
    }

    const core::Image& ImageResource::getImage() const noexcept
    {
        return m_Image;
    }

    core::Image& ImageResource::getImage() noexcept
    {
        return m_Image;
    }

    const core::DeviceMemory& ImageResource::getMemory() const noexcept
    {
        return m_Memory;
    }

    core::DeviceMemory& ImageResource::getMemory() noexcept
    {
        return m_Memory;
    }

    void ImageResource::callbackUpdateImageLayout(
        ImageResource* texture,
        u32 mipLevel,
        VkImageLayout newLayout
    )
    {
        texture->m_Image.setCurrentLayout(mipLevel, newLayout);
    }

    void ImageResource::callbackUpdateImageLayouts(
        ImageResource* texture,
        VkImageLayout newLayout
    )
    {
        texture->m_Image.setCurrentLayouts(newLayout);
    }
}
