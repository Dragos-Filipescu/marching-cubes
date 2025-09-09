#include <core/device.hpp>

namespace marching_cubes::core {

    Device::Device(
        VkDevice device,
        VkQueue graphicsQueue,
        VkQueue presentQueue
    ) noexcept
        : m_Device{ device },
        m_GraphicsQueue{ graphicsQueue },
        m_PresentQueue{ presentQueue }
    {
    }

    VkDevice Device::getWrapped() const noexcept {
        return m_Device;
    }

    VkQueue Device::getGraphicsQueue() const noexcept {
        return m_GraphicsQueue;
    }

    VkQueue Device::getPresentQueue() const noexcept {
        return m_PresentQueue;
    }
}

