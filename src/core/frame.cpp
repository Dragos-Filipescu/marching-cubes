#include <core/frame.hpp>

#include <vulkan/vulkan_core.h>

#include <utility>
#include <vector>

#include <core/helpers/descriptor.hpp>

namespace marching_cubes::core {

    Frame::Frame(
        VkDevice device,
        VkCommandBuffer commandBuffer,
        VkSemaphore imageAvailableSemaphore,
        VkFence inFlightFence,
        TransformBufferRReferenceType transformBuffer,
		GizmoBufferRReferenceType gizmoBuffer,
        VkDescriptorSet descriptorSet,
		VkDescriptorSet gizmoDescriptorSet,
        const std::vector<helpers::DescriptorWriteInfo>& infos
    )
        : m_CommandBuffer{ commandBuffer },
        m_ImageAvailableSemaphore{
            imageAvailableSemaphore,
            deleters::VkSemaphoreDeleter{ device },
        },
        m_InFlightFence{
            inFlightFence,
            deleters::VkFenceDeleter{ device },
        },
        m_TransformBuffer{ std::move(transformBuffer) },
        m_GizmoTransformBuffer{ std::move(gizmoBuffer) },
        m_DescriptorSet{
            helpers::populateDescriptorSet(
                device,
                descriptorSet,
                infos
            ),
        },
        m_GizmoDescriptorSet{
            helpers::populateDescriptorSet(
                device,
                gizmoDescriptorSet,
                infos
            ),
        }
    {
    }

    VkCommandBuffer Frame::getCommandBuffer() const noexcept
    {
        return m_CommandBuffer;
    }

    VkSemaphore Frame::getImageAvailableSemaphore() const noexcept
    {
        return m_ImageAvailableSemaphore;
    }

    VkFence Frame::getInFlightFence() const noexcept
    {
        return m_InFlightFence;
    }

    Frame::TransformBufferConstReferenceType Frame::getTransformBuffer() const noexcept
    {
        return m_TransformBuffer;
    }

    Frame::TransformBufferReferenceType Frame::getTransformBuffer() noexcept
    {
        return m_TransformBuffer;
    }

    Frame::GizmoBufferConstReferenceType Frame::getGizmoTransformBuffer() const noexcept
    {
        return m_GizmoTransformBuffer;
	}

    Frame::GizmoBufferReferenceType Frame::getGizmoTransformBuffer() noexcept
    {
        return m_GizmoTransformBuffer;
	}

    VkDescriptorSet Frame::getDescriptorSet() const noexcept
    {
        return m_DescriptorSet;
    }

    VkDescriptorSet Frame::getGizmoDescriptorSet() const noexcept
    {
        return m_GizmoDescriptorSet;
    }
}
