#pragma once
#ifndef MARCHING_CUBES_CORE_BUFFER_HPP
#define MARCHING_CUBES_CORE_BUFFER_HPP

#include <vulkan/vulkan_core.h>

#include <core/wrapper.hpp>

namespace marching_cubes::core {

    class Buffer final : public WrapperTraits<Buffer, VkBuffer> {
    public:

        Buffer() noexcept = default;

        Buffer(
            VkDevice device,
            VkBufferCreateInfo createInfo
        );

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        Buffer(Buffer&&) noexcept = default;
        Buffer& operator=(Buffer&&) noexcept = default;

        ~Buffer() = default;

        [[nodiscard]] const VkBufferCreateInfo& getCreateInfo() const noexcept;
        [[nodiscard]] VkBuffer getWrapped() const noexcept;
        [[nodiscard]] const VkMemoryRequirements& getMemoryRequirements() const noexcept;
        [[nodiscard]] VkDeviceSize getAllocationSize() const noexcept;
        [[nodiscard]] VkDeviceSize getAlignment() const noexcept;
        [[nodiscard]] VkDeviceSize getMemoryTypeBits() const noexcept;
        [[nodiscard]] VkDeviceSize alignUp(VkDeviceSize size) const noexcept;

    private:
        VkBufferCreateInfo          m_CreateInfo{};
        OwningWrapper<VkBuffer>     m_Buffer{};
        VkMemoryRequirements        m_MemoryRequirements{};
    };
}

#endif // MARCHING_CUBES_CORE_BUFFER_HPP
