#pragma once
#ifndef MARCHING_CUBES_CORE_JOB_MANAGER_HPP
#define MARCHING_CUBES_CORE_JOB_MANAGER_HPP

#include <vulkan/vulkan_core.h>

#include <concepts>
#include <memory>
#include <type_traits>
#include <vector>

#include <core/jobs.hpp>

#include <core/wrapper.hpp>

namespace marching_cubes::core::jobs {

	class JobManager final {
		using JobQueueType = std::vector<std::unique_ptr<IJob>>;
	public:

		JobManager() noexcept = default;

		JobManager(
			VkDevice device,
			VkCommandPool commandPool,
			VkQueue transferQueue
		);

		JobManager(const JobManager&) = delete;
		JobManager& operator=(const JobManager&) = delete;

		JobManager(JobManager&&) noexcept = default;
		JobManager& operator=(JobManager&&) noexcept = default;

		~JobManager() = default;
		
		[[nodiscard]] VkDevice getDevice() const noexcept;
		[[nodiscard]] VkCommandPool getCommandPool() const noexcept;
		[[nodiscard]] VkFence getFence() const noexcept;
		[[nodiscard]] const JobQueueType& getJobs() const noexcept;

		template<typename Job>
			requires std::derived_from<Job, IJob>
		JobManager& enqueue(Job&& job)
		{
			job.onEnqueue();
			m_Jobs.emplace_back(std::make_unique<std::remove_cvref_t<Job>>(std::forward<Job>(job)));
			return *this;
		}

		JobManager& flush() noexcept;

	private:
		// non-owned
		VkDevice						m_Device{};
		VkQueue							m_TransferQueue{};
		// owned
		OwningWrapper<VkCommandPool>	m_CommandPool{};
		OwningWrapper<VkFence>			m_TransferFence{};
		JobQueueType					m_Jobs{};
	};
}

#endif // !MARCHING_CUBES_CORE_JOB_MANAGER_HPP

