#pragma once
#ifndef MARCHING_CUBES_CORE_DETAIL_SHADER_BUILDER_HPP
#define MARCHING_CUBES_CORE_DETAIL_SHADER_BUILDER_HPP

#include <vulkan/vulkan.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <core/helpers/shader_module.hpp>
#include <core/wrapper.hpp>

#include <utils/bit_enum.hpp>

namespace marching_cubes::core::detail {

	enum class FixedStateBitFlags : std::uint8_t {
		None = 0,
		InputAssembly = (1 << 0),
		Tessellation = (1 << 1),
		Viewport = (1 << 2),
		Rasterization = (1 << 3),
		Multisample = (1 << 4),
		DepthStencil = (1 << 5),
		ColorBlend = (1 << 6),
		All = InputAssembly
		| Tessellation
		| Viewport
		| Rasterization
		| Multisample
		| DepthStencil
		| ColorBlend,
		BIT_ENUM_TAG,
	};

	INCLUDE_BIT_ENUM_BIT_OPS;
	INCLUDE_BIT_ENUM_COMPARISON;
}

namespace marching_cubes::core::builders {
	template<
		std::uint32_t				ShaderStageMask,
		bool						HasVertexInput,
		detail::FixedStateBitFlags	FixedState,
		bool						HasDynamicState
	>
	class GraphicsPipelineBuilder;
}

namespace marching_cubes::core::detail {

	struct ShaderInfo {
		std::string							path;
		const char*							pName = "main";
		VkShaderStageFlagBits				stage;
		VkPipelineShaderStageCreateFlags	flags = 0;
		const VkSpecializationInfo*			spec = nullptr;
		const void*							pNext = nullptr;
	};

	template<
		std::uint32_t		ShaderStageMask = 0,
		bool				HasVertexInput	= false,
		FixedStateBitFlags	FixedState		= FixedStateBitFlags::None,
		bool				HasDynamicState = false
	>
	struct ShaderBuilder final {

		static constexpr bool HasAnyShaderStage = ShaderStageMask != 0;

		template<VkShaderStageFlagBits Stage>
		static constexpr bool HasStage = (ShaderStageMask & Stage) != 0;

		template<VkShaderStageFlagBits NewStage>
		using WithShaderStage = ShaderBuilder<
			(ShaderStageMask | NewStage),
			HasVertexInput,
			FixedState,
			HasDynamicState
		>;

		using WithVertexInput = ShaderBuilder<
			ShaderStageMask,
			true,
			FixedState,
			HasDynamicState
		>;

		ShaderBuilder(
			VkDevice		device,
			VkRenderPass	renderPass,
			std::size_t		maxExpectedShaders = 5
		)
			: m_Device{ device },
			m_RenderPass{ renderPass },
			m_ShaderModules{},
			m_ShaderStages{},
			m_ShaderFilePaths{},
			m_ShaderAllocators{},
			m_VertexInputState{}
		{
			m_ShaderStages.reserve(maxExpectedShaders);
			m_ShaderFilePaths.reserve(maxExpectedShaders);
			m_ShaderAllocators.reserve(maxExpectedShaders);
		}

		ShaderBuilder(const ShaderBuilder&) = delete;
		ShaderBuilder& operator=(const ShaderBuilder&) = delete;

		ShaderBuilder(ShaderBuilder&&) noexcept = default;
		ShaderBuilder& operator=(ShaderBuilder&&) noexcept = default;

		template<
			std::uint32_t		OtherShaderStageMask,
			bool				OtherHasVertexInput,
			FixedStateBitFlags	OtherFixedState,
			bool				OtherHasDynamicState
		>
		ShaderBuilder(
			ShaderBuilder<
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
			m_ShaderFilePaths{ std::move(other.m_ShaderFilePaths) },
			m_ShaderAllocators{ std::move(other.m_ShaderAllocators) },
			m_VertexInputState{ other.m_VertexInputState }
		{
		}

		template<
			std::uint32_t		OtherShaderStageMask,
			bool				OtherHasVertexInput,
			FixedStateBitFlags	OtherFixedState,
			bool				OtherHasDynamicState
		>
		ShaderBuilder& operator=(
			ShaderBuilder<
				OtherShaderStageMask,
				OtherHasVertexInput,
				OtherFixedState,
				OtherHasDynamicState
			>&& other
			) noexcept {
			if (this != &other) {
				m_Device			= other.m_Device;
				m_RenderPass		= other.m_RenderPass;
				m_ShaderModules		= std::move(other.m_ShaderModules);
				m_ShaderStages		= std::move(other.m_ShaderStages);
				m_ShaderFilePaths	= std::move(other.m_ShaderFilePaths);
				m_ShaderAllocators	= std::move(other.m_ShaderAllocators);
				m_VertexInputState	= other.m_VertexInputState;
			}
			return *this;
		}

		~ShaderBuilder() noexcept = default;

		template<VkShaderStageFlagBits ShaderStage>
			requires (!HasStage<ShaderStage>)
		[[nodiscard]] auto addShader(
			const std::string&					filePath,
			const char*							pName				= "main",
			VkPipelineShaderStageCreateFlags	flags				= 0,
			const VkSpecializationInfo*			specializationInfo	= nullptr,
			const void*							pNext				= nullptr,
			const VkAllocationCallbacks*		pAllocator			= nullptr
		) && -> WithShaderStage<ShaderStage>
		{
			m_ShaderStages.emplace_back(
				VkPipelineShaderStageCreateInfo{
					.sType					= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
					.pNext					= pNext,
					.flags					= flags,
					.stage					= ShaderStage,
					.module					= VK_NULL_HANDLE,
					.pName					= pName,
					.pSpecializationInfo	= specializationInfo,
				}
				);
			m_ShaderFilePaths.emplace_back(filePath);
			m_ShaderAllocators.emplace_back(pAllocator);
			return WithShaderStage<ShaderStage> { std::move(*this) };
		}

		[[nodiscard]] auto addVertexInput(
			const std::vector<VkVertexInputBindingDescription>&		bindingDescriptions,
			const std::vector<VkVertexInputAttributeDescription>&	attributeDescriptions,
			VkPipelineVertexInputStateCreateFlags					flags = 0,
			const void*												pNext = nullptr
		) && noexcept -> WithVertexInput
			requires (!HasVertexInput && HasStage<VK_SHADER_STAGE_VERTEX_BIT>)
		{
			m_VertexInputState = {
				.sType								= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				.pNext								= pNext,
				.flags								= flags,
				.vertexBindingDescriptionCount		= static_cast<std::uint32_t>(bindingDescriptions.size()),
				.pVertexBindingDescriptions			= bindingDescriptions.data(),
				.vertexAttributeDescriptionCount	= static_cast<std::uint32_t>(attributeDescriptions.size()),
				.pVertexAttributeDescriptions		= attributeDescriptions.data(),
			};
			return WithVertexInput{ std::move(*this) };
		}

		[[nodiscard]] auto withLayout(
			const std::vector<VkDescriptorSetLayout>&	descriptorSetLayouts,
			const std::vector<VkPushConstantRange>&		pushConstantRanges = {},
			VkPipelineLayoutCreateFlags					flags = 0,
			const void*									pNext = nullptr
		) && noexcept -> builders::GraphicsPipelineBuilder<
			ShaderStageMask,
			HasVertexInput,
			FixedState,
			HasDynamicState
		>
			requires (HasAnyShaderStage)
		{
			m_ShaderModules.reserve(static_cast<std::size_t>(std::popcount(ShaderStageMask)));
			for (std::size_t i = 0; i < m_ShaderStages.size(); i++) {
				auto shaderCode = marching_cubes::readFile(m_ShaderFilePaths[i]);

				const auto& shaderModule = m_ShaderModules.emplace_back(
					helpers::createShaderModule(
						m_Device,
						shaderCode,
						m_ShaderAllocators[i]
					),
					deleters::VkShaderModuleDeleter{ m_Device }
				);
				m_ShaderStages[i].module = shaderModule;
			}

			return builders::GraphicsPipelineBuilder<
				ShaderStageMask,
				HasVertexInput,
				FixedStateBitFlags::None,
				false
			> {
				std::move(*this),
				VkPipelineLayoutCreateInfo{
					.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
					.pNext					= pNext,
					.flags					= flags,
					.setLayoutCount			= static_cast<std::uint32_t>(descriptorSetLayouts.size()),
					.pSetLayouts			= descriptorSetLayouts.data(),
					.pushConstantRangeCount = static_cast<std::uint32_t>(pushConstantRanges.size()),
					.pPushConstantRanges	= pushConstantRanges.data(),
				},
			};
		}

		VkDevice										m_Device{};
		VkRenderPass									m_RenderPass{};
		std::vector<OwningWrapper<VkShaderModule>>		m_ShaderModules{};
		std::vector<VkPipelineShaderStageCreateInfo>	m_ShaderStages{};
		std::vector<std::string>						m_ShaderFilePaths{};
		std::vector<const VkAllocationCallbacks*>		m_ShaderAllocators{};
		VkPipelineVertexInputStateCreateInfo			m_VertexInputState{};
	};

	ShaderBuilder(VkDevice, VkRenderPass)->ShaderBuilder<>;
	ShaderBuilder(VkDevice, VkRenderPass, std::size_t)->ShaderBuilder<>;
}

#endif // !MARCHING_CUBES_CORE_DETAIL_SHADER_BUILDER_HPP
