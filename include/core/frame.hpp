#pragma once
#ifndef MARCHING_CUBES_CORE_FRAME_HPP
#define MARCHING_CUBES_CORE_FRAME_HPP

#include <vulkan/vulkan_core.h>

#include <vector>

#include <core/wrapper.hpp>
#include <core/helpers/descriptor.hpp>

#include <resources/object_buffer.hpp>

#include <scene/ubo.hpp>

namespace marching_cubes::core {

	class Frame final {
	public:

		using TransformBufferType = resources::DirectObjectBuffer<scene::ModelViewProjectionUBO>;
		using TransformBufferReferenceType = std::add_lvalue_reference_t<TransformBufferType>;
		using TransformBufferConstReferenceType = std::add_lvalue_reference_t<std::add_const_t<TransformBufferType>>;
		using TransformBufferRReferenceType = std::add_rvalue_reference_t<TransformBufferType>;

		using GizmoBufferType = resources::DirectObjectBuffer<scene::ModelViewProjectionUBO>;
		using GizmoBufferReferenceType = std::add_lvalue_reference_t<GizmoBufferType>;
		using GizmoBufferConstReferenceType = std::add_lvalue_reference_t<std::add_const_t<GizmoBufferType>>;
		using GizmoBufferRReferenceType = std::add_rvalue_reference_t<GizmoBufferType>;

		Frame() noexcept = default;

		Frame(
			VkDevice device,
			VkCommandBuffer commandBuffer,
			VkSemaphore imageAvailableSemaphore,
			VkFence inFlightFence,
			TransformBufferRReferenceType transformBuffer,
			GizmoBufferRReferenceType gizmoBuffer,
			VkDescriptorSet descriptorSet,
			VkDescriptorSet gizmoDescriptorSet,
			const std::vector<core::helpers::DescriptorWriteInfo>& infos
		);

		Frame(const Frame&) = delete;
		Frame& operator=(const Frame&) = delete;

		Frame(Frame&&) noexcept = default;
		Frame& operator=(Frame&&) noexcept = default;

		~Frame() = default;

		[[nodiscard]] VkCommandBuffer getCommandBuffer() const noexcept;
		[[nodiscard]] VkSemaphore getImageAvailableSemaphore() const noexcept;
		[[nodiscard]] VkFence getInFlightFence() const noexcept;
		[[nodiscard]] TransformBufferConstReferenceType getTransformBuffer() const noexcept;
		[[nodiscard]] TransformBufferReferenceType getTransformBuffer() noexcept;
		[[nodiscard]] GizmoBufferConstReferenceType getGizmoTransformBuffer() const noexcept;
		[[nodiscard]] GizmoBufferReferenceType getGizmoTransformBuffer() noexcept;
		[[nodiscard]] VkDescriptorSet getDescriptorSet() const noexcept;
		[[nodiscard]] VkDescriptorSet getGizmoDescriptorSet() const noexcept;

	private:
		// non-owned
		VkCommandBuffer				m_CommandBuffer{};
		// owned
		OwningWrapper<VkSemaphore>	m_ImageAvailableSemaphore{};
		OwningWrapper<VkFence>		m_InFlightFence{};
		TransformBufferType			m_TransformBuffer{};
		GizmoBufferType				m_GizmoTransformBuffer{};
		VkDescriptorSet				m_DescriptorSet{};
		VkDescriptorSet 			m_GizmoDescriptorSet{};
	};
}
#endif // !MARCHING_CUBES_CORE_FRAME_HPP
