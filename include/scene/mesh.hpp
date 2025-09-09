#pragma once
#ifndef MARCHING_CUBES_SCENE_MESH_HPP
#define MARCHING_CUBES_SCENE_MESH_HPP

#include <vulkan/vulkan_core.h>

#include <cstddef>
#include <cstdint>

#include <core/aliases.hpp>
#include <resources/object_buffer.hpp>
#include <scene/interfaces.hpp>
#include <scene/vertex.hpp>

namespace marching_cubes::scene {

	template<Vertex VertexT>
	class Mesh final : public IMesh {

		using VertexBufferType = resources::StagedObjectBuffer<VertexT>;
		using IndexBufferType = resources::StagedObjectBuffer<u32>;

	public:

		using VertexType = VertexT;
		using IndexType = u32;

		Mesh() noexcept = default;

		Mesh(
			VkDevice device,
			VkPhysicalDevice physicalDevice,
			const VkBufferCreateInfo& vertexBufferCreateInfo,
			const VkBufferCreateInfo& indexBufferCreateInfo
		)
			: m_VertexBuffer{
				device,
				physicalDevice,
				vertexBufferCreateInfo,
			},
			m_IndexBuffer{
				device,
				physicalDevice,
				indexBufferCreateInfo,
			}
		{
		}

		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;

		Mesh(Mesh&&) noexcept = default;
		Mesh& operator=(Mesh&&) noexcept = default;

		~Mesh() = default;

		[[nodiscard]] const VertexBufferType& getVertexBuffer() const noexcept {
			return m_VertexBuffer;
		}

		[[nodiscard]] VertexBufferType& getVertexBuffer() noexcept {
			return m_VertexBuffer;
		}

		[[nodiscard]] const IndexBufferType& getIndexBuffer() const noexcept {
			return m_IndexBuffer;
		}

		[[nodiscard]] IndexBufferType& getIndexBuffer() noexcept {
			return m_IndexBuffer;
		}

		void bind(VkCommandBuffer cmd) const noexcept {
			VkDeviceSize offsets[] = { 0 };
			const VkBuffer vertexBuffer = m_VertexBuffer.getBuffer();
			vkCmdBindVertexBuffers(
				cmd,
				0,
				1,
				&vertexBuffer,
				offsets
			);
			vkCmdBindIndexBuffer(
				cmd,
				m_IndexBuffer.getBuffer(),
				0,
				VK_INDEX_TYPE_UINT32
			);
		}

		void draw(
			VkCommandBuffer cmd,
			u32 instanceCount,
			u32 firstInstance
		) const noexcept {
			vkCmdDrawIndexed(
				cmd,
				static_cast<u32>(m_IndexBuffer.getElementCount()),
				instanceCount,
				0,
				0,
				firstInstance
			);
		}

		[[nodiscard]] VkVertexInputBindingDescription getBindingDescription(u32 binding) const noexcept override
		{
			return VertexType::GetBindingDescription(binding);
		}

		[[nodiscard]] std::span<const VkVertexInputAttributeDescription> getAttributeDescriptions(u32 binding) const noexcept override
		{
			return VertexType::GetAttributeDescriptions(binding);
		}

		[[nodiscard]] std::size_t indexCount() const noexcept override
		{
			return m_IndexBuffer.getElementCount();
		}

	private:
		VertexBufferType	m_VertexBuffer{};
		IndexBufferType		m_IndexBuffer{};
	};
}

#endif // !MARCHING_CUBES_SCENE_MESH_HPP

