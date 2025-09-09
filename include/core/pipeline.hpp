#pragma once
#ifndef MARCHING_CUBES_CORE_PIPELINE_HPP
#define MARCHING_CUBES_CORE_PIPELINE_HPP

#include <vulkan/vulkan_core.h>

#include <core/wrapper.hpp>

namespace marching_cubes::core {

	class Pipeline final : public WrapperTraits<Pipeline, VkPipeline> {
	public:
		
		enum class PipelineType : uint8_t {
			GraphicsPipeline = 0,
			ComputePipeline,
		};

		Pipeline() noexcept = default;

		Pipeline(
			VkDevice device,
			VkPipelineLayout pipelineLayout,
			VkPipeline pipeline,
			PipelineType type
		) noexcept;

		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;

		Pipeline(Pipeline&&) noexcept = default;
		Pipeline& operator=(Pipeline&&) noexcept = default;

		~Pipeline() = default;

		[[nodiscard]] VkPipelineLayout getPipelineLayout() const noexcept;
		[[nodiscard]] VkPipeline getWrapped() const noexcept;
		[[nodiscard]] PipelineType getType() const noexcept;

	private:
		// owned
		OwningWrapper<VkPipelineLayout> m_PipelineLayout{};
		OwningWrapper<VkPipeline>		m_Pipeline{};
		PipelineType					m_Type{};
	};
}
#endif // !MARCHING_CUBES_CORE_PIPELINE_HPP
