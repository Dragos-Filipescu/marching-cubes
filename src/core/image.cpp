#include <core/image.hpp>

#include <stdexcept>

#include <core/helpers/image.hpp>

namespace marching_cubes::core {

    Image::Image(
        VkDevice device,
        VkImageCreateInfo imageCreateInfo
    )
        : m_CreateInfo{ imageCreateInfo },
        m_CurrentLayouts{
            { 0, imageCreateInfo.initialLayout }
        },
        m_Image{
            helpers::createImage(device, imageCreateInfo),
            deleters::VkImageDeleter{ device },
        },
        m_MemoryRequirements{
            helpers::getImageMemoryRequirements(device, m_Image)
        }
    {
        for (uint32_t i = 1; i < imageCreateInfo.mipLevels; i++) {
            m_CurrentLayouts[i] = VK_IMAGE_LAYOUT_UNDEFINED;
        }
    }

    const VkImageCreateInfo& Image::getCreateInfo() const noexcept {
        return m_CreateInfo;
    }

    Image::LayoutsConstReference Image::getCurrentLayouts() const noexcept {
        return m_CurrentLayouts;
    }

    VkImage Image::getWrapped() const noexcept {
        return m_Image;
    }

    const VkMemoryRequirements& Image::getMemoryRequirements() const noexcept {
        return m_MemoryRequirements;
    }

    VkDeviceSize Image::getAllocationSize() const noexcept {
        return m_MemoryRequirements.size;
    }

    VkDeviceSize Image::getAlignment() const noexcept {
        return m_MemoryRequirements.alignment;
    }

    VkDeviceSize Image::getMemoryTypeBits() const noexcept {
        return m_MemoryRequirements.memoryTypeBits;
    }

    VkDeviceSize Image::alignUp(VkDeviceSize size) const noexcept {
        return (size + m_MemoryRequirements.alignment - 1) & ~(m_MemoryRequirements.alignment - 1);
    }

    Image& Image::setCurrentLayout(uint32_t mipLevel, VkImageLayout newLayout) {
        m_CurrentLayouts[mipLevel] = newLayout;
        return *this;
    }

    Image& Image::setCurrentLayouts(VkImageLayout newLayout) {
        for (auto& [mipLevel, layout] : m_CurrentLayouts) {
            layout = newLayout;
        }
        return *this;
    }
}
