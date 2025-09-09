#pragma once
#ifndef MARCHING_CUBES_CORE_BUILDERS_COMPUTE_PIPELINE_BUILDER_HPP
#define MARCHING_CUBES_CORE_BUILDERS_COMPUTE_PIPELINE_BUILDER_HPP

#include <vulkan/vulkan.h>

#include <cstdint>
#include <string>
#include <vector>

#include <core/helpers/pipeline.hpp>
#include <core/helpers/shader_module.hpp>
#include <core/pipeline.hpp>
#include <core/wrapper.hpp>

#include <utils/utils.hpp>

namespace marching_cubes::core::builders {

	template<
		bool HasShader = false,
		bool HasLayout = false
	>
	class ComputePipelineBuilder final {

		using WithShader = ComputePipelineBuilder<true, HasLayout>;
		using WithLayout = ComputePipelineBuilder<HasShader, true>;

	public:

		ComputePipelineBuilder(
			VkDevice device,
			VkRenderPass renderPass
		)
			: m_Device{ device },
			m_RenderPass{ renderPass },
			m_ShaderModule{},
			m_StageInfo{},
			m_PipelineLayoutInfo{},
			m_PipelineLayout{}
		{
		}

		[[nodiscard]] auto setShader(
			const std::string&					filePath,
			const char*							pName				= "main",
			VkPipelineShaderStageCreateFlags	flags				= 0,
			const VkSpecializationInfo*			specializationInfo	= nullptr,
			const void*							pNext				= nullptr
		) && -> WithShader {
			auto shaderCode = marching_cubes::readFile(filePath);

			const auto& shaderModule = OwningWrapper{
				helpers::createShaderModule(m_Device, shaderCode),
				deleters::VkShaderModuleDeleter{ m_Device },
			};
			m_StageInfo = VkPipelineShaderStageCreateInfo{
				.sType					= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.pNext					= pNext,
				.flags					= flags,
				.stage					= VK_SHADER_STAGE_COMPUTE_BIT,
				.module					= shaderModule,
				.pName					= pName,
				.pSpecializationInfo	= specializationInfo,
			};
			return WithShader{ std::move(*this) };
		}

		[[nodiscard]] auto withLayout(
			const std::vector<VkDescriptorSetLayout>&	setLayouts,
			const std::vector<VkPushConstantRange>&		pushRanges = {},
			VkPipelineLayoutCreateFlags					flags = 0,
			const void*									pNext = nullptr
		) && -> WithLayout {
			m_PipelineLayoutInfo = VkPipelineLayoutCreateInfo{
			  .sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			  .pNext					= pNext,
			  .flags					= flags,
			  .setLayoutCount			= static_cast<std::uint32_t>(setLayouts.size()),
			  .pSetLayouts				= setLayouts.data(),
			  .pushConstantRangeCount	= static_cast<std::uint32_t>(pushRanges.size()),
			  .pPushConstantRanges		= pushRanges.data()
			};
			return WithLayout{ std::move(*this) };
		}

		[[nodiscard]] Pipeline build(
			VkPipelineCache					pipelineCache = VK_NULL_HANDLE,
			VkPipeline						basePipelineHandle = VK_NULL_HANDLE,
			std::int32_t					basePipelineIndex = -1,
			VkPipelineCreateFlags			flags = 0,
			const void*						pNext = nullptr,
			const VkAllocationCallbacks*	pAllocator = nullptr
		) &&
			requires(HasShader&& HasLayout)
		{
			VkComputePipelineCreateInfo createInfo = getCreateInfo(
				basePipelineHandle,
				basePipelineIndex,
				flags,
				pNext,
				pAllocator
			);

			auto pipelines = helpers::createComputePipelines(
				m_Device,
				std::vector{ createInfo },
				pipelineCache,
				pAllocator
			);

			return Pipeline{
				m_Device,
				m_PipelineLayout,
				pipelines[0],
				Pipeline::PipelineType::ComputePipeline
			};
		}

	private:

		[[nodiscard]] VkGraphicsPipelineCreateInfo getCreateInfo(
			VkPipeline						basePipelineHandle = VK_NULL_HANDLE,
			std::int32_t					basePipelineIndex = -1,
			VkPipelineCreateFlags			flags = 0,
			const void*						pNext = nullptr,
			const VkAllocationCallbacks*	pAllocator = nullptr
		) requires (HasShader&& HasLayout)
		{
			if (m_PipelineLayout == VK_NULL_HANDLE) {
				m_PipelineLayout = helpers::createPipelineLayout(
					m_Device,
					m_PipelineLayoutInfo,
					pAllocator
				);
			}

			return VkComputePipelineCreateInfo{
			  .sType				= VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			  .pNext				= pNext,
			  .flags				= flags,
			  .stage				= m_StageInfo,
			  .layout				= m_PipelineLayout,
			  .basePipelineHandle	= basePipelineHandle,
			  .basePipelineIndex	= basePipelineIndex
			};
		}

		VkDevice						m_Device{};
		VkRenderPass					m_RenderPass{};

		OwningWrapper<VkShaderModule>	m_ShaderModule{};
		VkPipelineShaderStageCreateInfo m_StageInfo{};
		VkPipelineLayoutCreateInfo		m_PipelineLayoutInfo{};
		VkPipelineLayout				m_PipelineLayout{};
	};

	ComputePipelineBuilder(VkDevice, VkRenderPass)->ComputePipelineBuilder<>;

}

#endif // !MARCHING_CUBES_CORE_BUILDERS_COMPUTE_PIPELINE_BUILDER_HPP

