#include <core/pipeline.hpp>

namespace marching_cubes::core {

	Pipeline::Pipeline(
		VkDevice device,
		VkPipelineLayout pipelineLayout,
		VkPipeline pipeline,
		PipelineType type
	) noexcept
		: m_PipelineLayout{
			pipelineLayout,
			deleters::VkPipelineLayoutDeleter{ device },
		},
		m_Pipeline{
			pipeline,
			deleters::VkPipelineDeleter{ device },
		},
		m_Type{ type }
	{
	}

	VkPipelineLayout Pipeline::getPipelineLayout() const noexcept {
		return m_PipelineLayout;
	}

	VkPipeline Pipeline::getWrapped() const noexcept {
		return m_Pipeline;
	}

	Pipeline::PipelineType Pipeline::getType() const noexcept {
		return m_Type;
	}
}
