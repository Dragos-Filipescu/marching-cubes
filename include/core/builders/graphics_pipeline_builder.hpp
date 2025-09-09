#pragma once
#ifndef MARCHING_CUBES_CORE_BUILDERS_PIPELINE_BUILDER_HPP
#define MARCHING_CUBES_CORE_BUILDERS_PIPELINE_BUILDER_HPP

#include <vulkan/vulkan.h>

#include <bit>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <core/builders/detail/shader_builder.hpp>
#include <core/helpers/pipeline.hpp>
#include <core/helpers/shader_module.hpp>
#include <core/pipeline.hpp>
#include <core/wrapper.hpp>

#include <utils/bit_enum.hpp>

namespace marching_cubes::core::builders {

	INCLUDE_BIT_ENUM_BIT_OPS;
	INCLUDE_BIT_ENUM_COMPARISON;

	template<std::uint32_t M>
	concept HasShaders = (M != 0);

	template<
		std::uint32_t				ShaderStageMask = 0,
		bool						HasVertexInput = false,
		detail::FixedStateBitFlags	FixedState = detail::FixedStateBitFlags::None,
		bool						HasDynamicState = false
	>
	class GraphicsPipelineBuilder final {

		static constexpr bool HasAnyShaderStage		= ShaderStageMask									!= 0;
		static constexpr bool HasVertexShaderStage	= (ShaderStageMask & VK_SHADER_STAGE_VERTEX_BIT)	!= 0;

		using FixedStateBitFlags = detail::FixedStateBitFlags;

		template<
			std::uint32_t		ShaderBuilderStageMask = 0,
			bool				ShaderBuilderHasVertexInput = false,
			FixedStateBitFlags	ShaderBuilderFixedState = FixedStateBitFlags::None,
			bool				ShaderBuilderHasDynamicState = false
		>
		using ShaderBuilder = detail::ShaderBuilder<
			ShaderBuilderStageMask,
			ShaderBuilderHasVertexInput,
			ShaderBuilderFixedState,
			ShaderBuilderHasDynamicState
		>;

		template<FixedStateBitFlags StateFlag>
		static constexpr bool HasFixedState = (FixedState & StateFlag) != FixedStateBitFlags::None;

		template<
			std::uint32_t		OtherShaderStageMask,
			bool				OtherHasVertexInput,
			FixedStateBitFlags	OtherFixedState,
			bool				OtherHasDynamicState
		>
		friend class GraphicsPipelineBuilder;

		template<
			std::uint32_t		ShaderBuilderShaderStageMask,
			bool				ShaderBuilderHasVertexInput,
			FixedStateBitFlags	ShaderBuilderFixedState,
			bool				ShaderBuilderHasDynamicState
		>
		friend struct detail::ShaderBuilder;

		template <FixedStateBitFlags NewFixedState>
		using WithFixedState = GraphicsPipelineBuilder<
			ShaderStageMask,
			HasVertexInput,
			FixedState | NewFixedState,
			HasDynamicState
		>;

		using WithDynamicState = GraphicsPipelineBuilder<
			ShaderStageMask,
			HasVertexInput,
			FixedState,
			true
		>;

	public:
		GraphicsPipelineBuilder(
			VkDevice		device,
			VkRenderPass	renderPass
		) noexcept
			: m_Device{ device },
			m_RenderPass{ renderPass },
			m_ShaderModules{},
			m_ShaderStages{},
			m_VertexInputState{},
			m_InputAssemblyState{},
			m_TessellationState{},
			m_ViewportState{},
			m_RasterizationState{},
			m_MultisampleState{},
			m_DepthStencilState{},
			m_ColorBlendState{},
			m_DynamicState{},
			m_PipelineLayoutInfo{},
			m_PipelineLayout{}
		{
		}

		template<
			std::uint32_t		ShaderBuilderStageMask,
			bool				ShaderBuilderHasVertexInput,
			FixedStateBitFlags	ShaderBuilderFixedState,
			bool				ShaderBuilderHasDynamicState
		>
		GraphicsPipelineBuilder(
			ShaderBuilder<
				ShaderBuilderStageMask,
				ShaderBuilderHasVertexInput,
				ShaderBuilderFixedState,
				ShaderBuilderHasDynamicState
			>&&									shaderBuilder,
			const VkPipelineLayoutCreateInfo&	pipelineLayoutCreateInfo
		) noexcept
			: m_Device{ shaderBuilder.m_Device },
			m_RenderPass{ shaderBuilder.m_RenderPass },
			m_ShaderModules{ std::move(shaderBuilder.m_ShaderModules) },
			m_ShaderStages{ std::move(shaderBuilder.m_ShaderStages) },
			m_VertexInputState{ shaderBuilder.m_VertexInputState },
			m_InputAssemblyState{},
			m_TessellationState{},
			m_ViewportState{},
			m_RasterizationState{},
			m_MultisampleState{},
			m_DepthStencilState{},
			m_ColorBlendState{},
			m_DynamicState{},
			m_PipelineLayoutInfo{ pipelineLayoutCreateInfo },
			m_PipelineLayout{}
		{
		}

		GraphicsPipelineBuilder(const GraphicsPipelineBuilder&)				= delete;
		GraphicsPipelineBuilder& operator=(const GraphicsPipelineBuilder&)	= delete;

		GraphicsPipelineBuilder(GraphicsPipelineBuilder&&)				= default;
		GraphicsPipelineBuilder& operator=(GraphicsPipelineBuilder&&)	= default;

		template<
			std::uint32_t		OtherShaderStageMask,
			bool				OtherHasVertexInput,
			FixedStateBitFlags	OtherFixedState,
			bool				OtherHasDynamicState
		>
		GraphicsPipelineBuilder(
			GraphicsPipelineBuilder<
				OtherShaderStageMask,
				OtherHasVertexInput,
				OtherFixedState,
				OtherHasDynamicState
			>&& other
		) noexcept
			: m_Device{ other.m_Device },
			m_RenderPass{ other.m_RenderPass },
			m_ShaderModules{ std::move(other.m_ShaderModules) },
			m_ShaderStages{ std::move(other.m_ShaderStages) },
			m_VertexInputState{ other.m_VertexInputState },
			m_InputAssemblyState{ other.m_InputAssemblyState },
			m_TessellationState{ other.m_TessellationState },
			m_ViewportState{ other.m_ViewportState },
			m_RasterizationState{ other.m_RasterizationState },
			m_MultisampleState{ other.m_MultisampleState },
			m_DepthStencilState{ other.m_DepthStencilState },
			m_ColorBlendState{ other.m_ColorBlendState },
			m_DynamicState{ other.m_DynamicState },
			m_PipelineLayoutInfo{ other.m_PipelineLayoutInfo },
			m_PipelineLayout{ std::move(other.m_PipelineLayout) }
		{
		}

		template<
			std::uint32_t		OtherShaderStageMask,
			bool				OtherHasVertexInput,
			FixedStateBitFlags	OtherFixedState,
			bool				OtherHasDynamicState
		>
		GraphicsPipelineBuilder& operator=(
			GraphicsPipelineBuilder<
				OtherShaderStageMask,
				OtherHasVertexInput,
				OtherFixedState,
				OtherHasDynamicState
			>&& other
		) noexcept {
			if (this != &other) {
				m_Device				= other.m_Device;
				m_RenderPass			= other.m_RenderPass;
				m_ShaderModules			= std::move(other.m_ShaderModules);
				m_ShaderStages			= std::move(other.m_ShaderStages);
				m_VertexInputState		= other.m_VertexInputState;
				m_InputAssemblyState	= other.m_InputAssemblyState;
				m_TessellationState		= other.m_TessellationState;
				m_ViewportState			= other.m_ViewportState;
				m_RasterizationState	= other.m_RasterizationState;
				m_MultisampleState		= other.m_MultisampleState;
				m_DepthStencilState		= other.m_DepthStencilState;
				m_ColorBlendState		= other.m_ColorBlendState;
				m_DynamicState			= other.m_DynamicState;
				m_PipelineLayoutInfo	= other.m_PipelineLayoutInfo;
				m_PipelineLayout		= std::move(other.m_PipelineLayout);
			}
			return *this;
		}

		~GraphicsPipelineBuilder() noexcept = default;

		[[nodiscard]] auto withShaders(
			std::size_t maxExpectedShaders = 5
		) && -> ShaderBuilder<0, false, FixedState, HasDynamicState>
			requires (!HasAnyShaderStage)
		{
			return ShaderBuilder<0, false, FixedState, HasDynamicState> {
				m_Device,
				m_RenderPass,
				maxExpectedShaders,
			};
		}

		[[nodiscard]] auto addInputAssemblyState(
			const VkPipelineInputAssemblyStateCreateInfo& createInfo
		) && noexcept -> WithFixedState<FixedStateBitFlags::InputAssembly>
			requires (!HasFixedState<FixedStateBitFlags::InputAssembly>)
		{
			m_InputAssemblyState = createInfo;
			return WithFixedState<FixedStateBitFlags::InputAssembly> { std::move(*this) };
		}

		[[nodiscard]] auto addTessellationState(
			const VkPipelineTessellationStateCreateInfo& createInfo
		) && noexcept -> WithFixedState<FixedStateBitFlags::Tessellation>
			requires (!HasFixedState<FixedStateBitFlags::Tessellation>)
		{
			m_TessellationState = createInfo;
			return WithFixedState<FixedStateBitFlags::Tessellation> { std::move(*this) };
		}

		[[nodiscard]] auto addViewportState(
			const std::vector<VkViewport>&		viewports,
			const std::vector<VkRect2D>&		scissors,
			VkPipelineViewportStateCreateFlags	flags = 0,
			const void*							pNext = nullptr
		) && noexcept -> WithFixedState<FixedStateBitFlags::Viewport>
			requires (!HasFixedState<FixedStateBitFlags::Viewport>)
		{
			m_ViewportState = {
				.sType			= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
				.pNext			= pNext,
				.flags			= flags,
				.viewportCount	= static_cast<std::uint32_t>(viewports.size()),
				.pViewports		= viewports.data(),
				.scissorCount	= static_cast<std::uint32_t>(scissors.size()),
				.pScissors		= scissors.data(),
			};
			return WithFixedState<FixedStateBitFlags::Viewport> { std::move(*this) };
		}

		[[nodiscard]] auto addRasterizationState(
			const VkPipelineRasterizationStateCreateInfo& rasterizationState
		) && noexcept -> WithFixedState<FixedStateBitFlags::Rasterization>
			requires (!HasFixedState<FixedStateBitFlags::Rasterization>)
		{
			m_RasterizationState = rasterizationState;
			return WithFixedState<FixedStateBitFlags::Rasterization> { std::move(*this) };
		}

		[[nodiscard]] auto addMultisampleState(
			const VkPipelineMultisampleStateCreateInfo& multisampleState
		) && noexcept -> WithFixedState<FixedStateBitFlags::Multisample>
			requires (!HasFixedState<FixedStateBitFlags::Multisample>)
		{
			m_MultisampleState = multisampleState;
			return WithFixedState<FixedStateBitFlags::Multisample> { std::move(*this) };
		}

		[[nodiscard]] auto addDepthStencilState(
			const VkPipelineDepthStencilStateCreateInfo& depthStencilState
		) && noexcept -> WithFixedState<FixedStateBitFlags::DepthStencil>
			requires (!HasFixedState<FixedStateBitFlags::DepthStencil>)
		{
			m_DepthStencilState = depthStencilState;
			return WithFixedState<FixedStateBitFlags::DepthStencil> { std::move(*this) };
		}

		[[nodiscard]] auto addColorBlendState(
			const std::vector<VkPipelineColorBlendAttachmentState>& colorBlendAttachments,
			VkBool32												logicOpEnable	= VK_FALSE,
			VkLogicOp												logicOp			= VK_LOGIC_OP_COPY,
			std::array<float, 4>									blendConstants	= { { 0.0f, 0.0f, 0.0f, 0.0f } },
			VkPipelineColorBlendStateCreateFlags					flags			= 0,
			const void*												pNext			= nullptr
		) && noexcept -> WithFixedState<FixedStateBitFlags::ColorBlend>
			requires (!HasFixedState<FixedStateBitFlags::ColorBlend>)
		{
			m_ColorBlendState = {
				.sType				= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
				.pNext				= pNext,
				.flags				= flags,
				.logicOpEnable		= logicOpEnable,
				.logicOp			= logicOp,
				.attachmentCount	= static_cast<std::uint32_t>(colorBlendAttachments.size()),
				.pAttachments		= colorBlendAttachments.data(),
				.blendConstants		= { blendConstants[0], blendConstants[1], blendConstants[2], blendConstants[3] },
			};
			return WithFixedState<FixedStateBitFlags::ColorBlend> { std::move(*this) };
		}

		[[nodiscard]] auto addDynamicState(
			const std::vector<VkDynamicState>&	dynamicStates,
			VkPipelineDynamicStateCreateFlags	flags = 0,
			const void*							pNext = nullptr
		) && noexcept -> WithDynamicState
			requires (!HasDynamicState)
		{
			m_DynamicState = {
				.sType				= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
				.pNext				= pNext,
				.flags				= flags,
				.dynamicStateCount	= static_cast<std::uint32_t>(dynamicStates.size()),
				.pDynamicStates		= dynamicStates.data(),
			};
			return WithDynamicState{ std::move(*this) };
		}

		[[nodiscard]] Pipeline build(
			std::uint32_t					subpass				= 0,
			VkPipelineCache					pipelineCache		= VK_NULL_HANDLE,
			VkPipeline						basePipelineHandle	= VK_NULL_HANDLE,
			std::int32_t					basePipelineIndex	= -1,
			VkPipelineCreateFlags			flags				= 0,
			const void*						pNext				= nullptr,
			const VkAllocationCallbacks*	pAllocator			= nullptr
		) requires (HasAnyShaderStage)
		{
			const VkGraphicsPipelineCreateInfo pipelineInfo = getCreateInfo(
				subpass,
				basePipelineHandle,
				basePipelineIndex,
				flags,
				pNext,
				pAllocator
			);

			VkPipeline graphicsPipeline = helpers::createGraphicsPipelines(
				m_Device,
				std::vector{ pipelineInfo },
				pipelineCache,
				pAllocator
			)[0];

			return Pipeline{
				m_Device,
				pipelineInfo.layout,
				graphicsPipeline,
				Pipeline::PipelineType::GraphicsPipeline,
			};
		}

	private:

		[[nodiscard]] VkGraphicsPipelineCreateInfo getCreateInfo(
			std::uint32_t					subpass				= 0,
			VkPipeline						basePipelineHandle	= VK_NULL_HANDLE,
			std::int32_t					basePipelineIndex	= -1,
			VkPipelineCreateFlags			flags				= 0,
			const void*						pNext				= nullptr,
			const VkAllocationCallbacks*	pAllocator			= nullptr
		) requires (HasAnyShaderStage)
		{
			if (m_PipelineLayout == VK_NULL_HANDLE) {
				m_PipelineLayout = helpers::createPipelineLayout(m_Device, m_PipelineLayoutInfo, pAllocator);
			}

			VkGraphicsPipelineCreateInfo pipelineInfo{
				.sType					= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.pNext					= pNext,
				.flags					= flags,
				.stageCount				= static_cast<std::uint32_t>(m_ShaderStages.size()),
				.pStages				= m_ShaderStages.data(),
				.pVertexInputState		= nullptr,
				.pInputAssemblyState	= nullptr,
				.pTessellationState		= nullptr,
				.pViewportState			= nullptr,
				.pRasterizationState	= nullptr,
				.pMultisampleState		= nullptr,
				.pDepthStencilState		= nullptr,
				.pColorBlendState		= nullptr,
				.pDynamicState			= nullptr,
				.layout					= m_PipelineLayout,
				.renderPass				= m_RenderPass,
				.subpass				= subpass,
				.basePipelineHandle		= basePipelineHandle,
				.basePipelineIndex		= basePipelineIndex,
			};

			if constexpr (HasVertexInput) {
				pipelineInfo.pVertexInputState = &m_VertexInputState;
			}
			if constexpr (HasFixedState<FixedStateBitFlags::InputAssembly>) {
				pipelineInfo.pInputAssemblyState = &m_InputAssemblyState;
			}
			if constexpr (HasFixedState<FixedStateBitFlags::Tessellation>) {
				pipelineInfo.pTessellationState = &m_TessellationState;
			}
			if constexpr (HasFixedState<FixedStateBitFlags::Viewport>) {
				pipelineInfo.pViewportState = &m_ViewportState;
			}
			if constexpr (HasFixedState<FixedStateBitFlags::Rasterization>) {
				pipelineInfo.pRasterizationState = &m_RasterizationState;
			}
			if constexpr (HasFixedState<FixedStateBitFlags::Multisample>) {
				pipelineInfo.pMultisampleState = &m_MultisampleState;
			}
			if constexpr (HasFixedState<FixedStateBitFlags::DepthStencil>) {
				pipelineInfo.pDepthStencilState = &m_DepthStencilState;
			}
			if constexpr (HasFixedState<FixedStateBitFlags::ColorBlend>) {
				pipelineInfo.pColorBlendState = &m_ColorBlendState;
			}
			if constexpr (HasDynamicState) {
				pipelineInfo.pDynamicState = &m_DynamicState;
			}

			return pipelineInfo;
		}

		VkDevice										m_Device{};
		VkRenderPass									m_RenderPass{};

		std::vector<OwningWrapper<VkShaderModule>>		m_ShaderModules{};
		std::vector<VkPipelineShaderStageCreateInfo>	m_ShaderStages{};

		VkPipelineVertexInputStateCreateInfo			m_VertexInputState{};
		VkPipelineInputAssemblyStateCreateInfo			m_InputAssemblyState{};
		VkPipelineTessellationStateCreateInfo			m_TessellationState{};
		VkPipelineViewportStateCreateInfo				m_ViewportState{};
		VkPipelineRasterizationStateCreateInfo			m_RasterizationState{};
		VkPipelineMultisampleStateCreateInfo			m_MultisampleState{};
		VkPipelineDepthStencilStateCreateInfo			m_DepthStencilState{};
		VkPipelineColorBlendStateCreateInfo				m_ColorBlendState{};
		VkPipelineDynamicStateCreateInfo				m_DynamicState{};
		VkPipelineLayoutCreateInfo						m_PipelineLayoutInfo{};
		VkPipelineLayout								m_PipelineLayout{};
	};

	GraphicsPipelineBuilder(VkDevice, VkRenderPass)->GraphicsPipelineBuilder<>;
}

#endif // !MARCHING_CUBES_CORE_BUILDERS_PIPELINE_BUILDER_HPP

