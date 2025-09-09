#include <core/buffer.hpp>

#include <core/helpers/buffer.hpp>

namespace marching_cubes::core {

    Buffer::Buffer(
        VkDevice device,
        VkBufferCreateInfo createInfo
    )
        : m_CreateInfo{ createInfo },
        m_Buffer{
            helpers::createBuffer(device, createInfo),
            deleters::VkBufferDeleter{ device }
        },
        m_MemoryRequirements{
            helpers::getBufferMemoryRequirements(device, m_Buffer)
        }
    {
    }

    const VkBufferCreateInfo& Buffer::getCreateInfo() const noexcept
    {
        return m_CreateInfo;
    }

    VkBuffer Buffer::getWrapped() const noexcept
    {
        return m_Buffer;
    }

    const VkMemoryRequirements& Buffer::getMemoryRequirements() const noexcept
    {
        return m_MemoryRequirements;
    }

    VkDeviceSize Buffer::getAllocationSize() const noexcept
    {
        return m_MemoryRequirements.size;
    }

    VkDeviceSize Buffer::getAlignment() const noexcept
    {
        return m_MemoryRequirements.alignment;
    }

    VkDeviceSize Buffer::getMemoryTypeBits() const noexcept
    {
        return m_MemoryRequirements.memoryTypeBits;
    }

    VkDeviceSize Buffer::alignUp(VkDeviceSize size) const noexcept
    {
        return m_MemoryRequirements.alignment != 0
            ? (size + m_MemoryRequirements.alignment - 1) & ~(m_MemoryRequirements.alignment - 1)
            : size;
    }
}
