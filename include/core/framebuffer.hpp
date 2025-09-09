#pragma once
#ifndef MARCHING_CUBES_CORE_FRAMEBUFFER_HPP
#define MARCHING_CUBES_CORE_FRAMEBUFFER_HPP

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <span>
#include <vector>

#include <core/wrapper.hpp>
#include <core/helpers/framebuffer.hpp>

namespace marching_cubes::core {

	class FramebufferSet final {

		using FrameBufferListType = std::vector<OwningWrapper<VkFramebuffer>>;
		using FrameBufferListConstReferenceType = std::add_lvalue_reference_t<std::add_const_t<FrameBufferListType>>;

	public:

		FramebufferSet() noexcept = default;

		template<
			std::ranges::forward_range Range,
			typename Element = std::ranges::range_value_t<Range>
		>
			requires std::convertible_to<Element, VkImageView>
		FramebufferSet(
			VkDevice device,
			VkRenderPass renderPass,
			Range&& swapchainImageViews,
			VkImageView colorImageView,
			VkImageView depthImageView,
			VkExtent2D extent
		)
			: m_FrameBuffers{}
		{
			auto n = static_cast<std::size_t>(std::ranges::size(swapchainImageViews));
			m_FrameBuffers.reserve(n);

			for (const Element& swapchainImageView : swapchainImageViews) {
				std::vector<VkImageView> attachments;
				attachments.reserve(
					1ull
					+ (depthImageView != VK_NULL_HANDLE ? 1u : 0u)
					+ (colorImageView != VK_NULL_HANDLE ? 1u : 0u)
				);
				if (colorImageView != VK_NULL_HANDLE) {
					attachments.emplace_back(colorImageView);
				}
				attachments.emplace_back(VkImageView{ swapchainImageView });
				if (depthImageView != VK_NULL_HANDLE) {
					attachments.emplace_back(depthImageView);
				}
				auto createInfo = helpers::makeFramebufferCreateInfo(renderPass, attachments, extent);
				m_FrameBuffers.emplace_back(
					helpers::createFrameBuffer(device, createInfo),
					deleters::VkFramebufferDeleter{ device }
				);
			}
		}

		FramebufferSet(const FramebufferSet&) = delete;
		FramebufferSet& operator=(const FramebufferSet&) = delete;

		FramebufferSet(FramebufferSet&&) = default;
		FramebufferSet& operator=(FramebufferSet&&) = default;

		~FramebufferSet() = default;

		[[nodiscard]] constexpr FrameBufferListConstReferenceType getFramebuffers() const noexcept {
			return m_FrameBuffers;
		}

	private:
		// owned
		FrameBufferListType m_FrameBuffers{};
	};
}

#endif // !MARCHING_CUBES_CORE_FRAMEBUFFER_HPP

