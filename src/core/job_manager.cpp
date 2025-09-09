#include <core/job_manager.hpp>

#include <vulkan/vulkan_core.h>

#include <limits>
#include <memory>
#include <vector>

#include <core/aliases.hpp>
#include <core/detail/default_deleters.hpp>
#include <core/helpers/command_buffer.hpp>
#include <core/helpers/fence.hpp>
#include <core/jobs.hpp>

namespace marching_cubes::core::jobs {

    JobManager::JobManager(
        VkDevice device,
        VkCommandPool commandPool,
        VkQueue transferQueue
    )
        : m_Device{ device },
        m_TransferQueue{ transferQueue },
        m_CommandPool{
            commandPool,
            deleters::VkCommandPoolDeleter{ device },
        },
        m_TransferFence{
            helpers::createFence(
                device, VkFenceCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                    .flags = VK_FENCE_CREATE_SIGNALED_BIT
                }
            ),
            deleters::VkFenceDeleter{ device },
        },
        m_Jobs{}
    {
    }

    VkDevice JobManager::getDevice() const noexcept
    {
        return m_Device;
    }

    VkCommandPool JobManager::getCommandPool() const noexcept
    {
        return m_CommandPool;
    }

    VkFence JobManager::getFence() const noexcept
    {
        return m_TransferFence;
    }

    const JobManager::JobQueueType& JobManager::getJobs() const noexcept
    {
        return m_Jobs;
    }

    JobManager& JobManager::flush() noexcept
    {
        if (m_Jobs.empty()) {
            return *this;
		}
        const VkFence fence = m_TransferFence;
        vkWaitForFences(m_Device, 1, &fence, VK_TRUE, std::numeric_limits<u64>::max());
        vkResetFences(m_Device, 1, &fence);

        const auto commandBuffer = helpers::beginSingleTimeCommands(m_Device, m_CommandPool);

        for (const auto& job : m_Jobs) {
            job->record(commandBuffer).onComplete();
        }

        helpers::endSingleTimeCommands(m_Device, m_CommandPool, commandBuffer, m_TransferQueue, &fence);

        m_Jobs.clear();

        return *this;
    }
}
