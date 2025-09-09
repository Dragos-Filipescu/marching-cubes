#include <core/device_memory.hpp>

#include <core/helpers/device_memory.hpp>

namespace marching_cubes::core {

    DeviceMemory::DeviceMemory(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkMemoryRequirements memoryRequirements,
        VkMemoryPropertyFlags properties
    )
        : m_Properties{ properties },
        m_Memory{
            helpers::allocateDeviceMemory(
                device,
                physicalDevice,
                memoryRequirements,
                properties
            ),
            deleters::VkDeviceMemoryDeleter{ device }
        }
    {
    }

    VkMemoryPropertyFlags DeviceMemory::getProperties() const noexcept {
        return m_Properties;
    }

    VkDeviceMemory DeviceMemory::getWrapped() const noexcept {
        return m_Memory;
    }
}
