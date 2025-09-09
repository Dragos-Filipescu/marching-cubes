#pragma once
#ifndef MARCHING_CUBES_SWAPCHAIN_HPP
#define MARCHING_CUBES_SWAPCHAIN_HPP

#include <vulkan/vulkan_core.h>

#include <span>
#include <vector>

#include <core/wrapper.hpp>

namespace marching_cubes::core {

	class Swapchain final : public WrapperTraits<Swapchain, VkSwapchainKHR> {
	public:

		Swapchain() noexcept = default;

		Swapchain(
			VkDevice device,
			VkExtent2D swapChainExtent,
			VkFormat swapChainImageFormat,
			VkSwapchainKHR swapChain,
			VkRenderPass renderPass
		) noexcept;

		Swapchain(const Swapchain&) = delete;
		Swapchain& operator=(const Swapchain&) = delete;

		Swapchain(Swapchain&&) noexcept = default;
		Swapchain& operator=(Swapchain&&) noexcept = default;

		~Swapchain() = default;

		[[nodiscard]] VkSwapchainKHR getWrapped() const noexcept;
		[[nodiscard]] VkRenderPass getRenderPass() const noexcept;
		[[nodiscard]] VkExtent2D getExtent() const noexcept;
		[[nodiscard]] VkFormat getImageFormat() const noexcept;
		[[nodiscard]] const std::vector<VkImage>& getImages() const noexcept;
		[[nodiscard]] const std::vector<OwningWrapper<VkImageView>>& getImageViews() const noexcept;
		[[nodiscard]] const std::vector<OwningWrapper<VkSemaphore>>& getImageRenderFinishedSemaphores() const noexcept;

	private:
		// owned
		VkExtent2D								m_SwapChainExtent{};
		VkFormat								m_SwapChainImageFormat{};
		OwningWrapper<VkSwapchainKHR>			m_Swapchain{};
		OwningWrapper<VkRenderPass>				m_RenderPass{};
		std::vector<VkImage>					m_SwapChainImages{};
		std::vector<OwningWrapper<VkImageView>> m_SwapChainImageViews{};
		std::vector<OwningWrapper<VkSemaphore>> m_ImageRenderFinishedSemaphores{};
	};
}
#endif // !MARCHING_CUBES_SWAPCHAIN_HPP
