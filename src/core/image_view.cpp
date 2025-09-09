#include <core/image_view.hpp>

#include <core/helpers/image_view.hpp>

namespace marching_cubes::core {

    ImageView::ImageView(
        VkDevice device,
        VkImageViewCreateInfo createInfo
    )
        : m_ImageView{
            helpers::createImageView(device, createInfo),
            deleters::VkImageViewDeleter{ device }
        },
        m_CreateInfo{ createInfo }
    {
    }

    VkImageView ImageView::getWrapped() const noexcept {
        return m_ImageView;
    }

    const VkImageViewCreateInfo& ImageView::getCreateInfo() const noexcept {
        return m_CreateInfo;
    }
}
