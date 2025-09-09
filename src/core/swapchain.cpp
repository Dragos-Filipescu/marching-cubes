#include "core/swapchain.hpp"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <utility>

#include <core/helpers/image_view.hpp>
#include <core/helpers/swapchain.hpp>

namespace marching_cubes::core {

	Swapchain::Swapchain(
		VkDevice device,
		VkExtent2D swapChainExtent,
		VkFormat swapChainImageFormat,
		VkSwapchainKHR swapChain,
		VkRenderPass renderPass
	) noexcept
		: m_SwapChainExtent{ swapChainExtent },
		m_SwapChainImageFormat{ swapChainImageFormat },
		m_Swapchain{
			swapChain,
			deleters::VkSwapchainKHRDeleter{ device },
		},
		m_RenderPass{
			renderPass,
			deleters::VkRenderPassDeleter{ device },
		},
		m_SwapChainImages{ helpers::getSwapChainImages(device, m_Swapchain) },
		m_SwapChainImageViews{ helpers::createImageViews(device, m_SwapChainImages, m_SwapChainImageFormat) },
		m_ImageRenderFinishedSemaphores{ helpers::createRenderFinishedSemaphores(device, m_SwapChainImages.size()) }
	{
	}

	VkSwapchainKHR Swapchain::getWrapped() const noexcept {
		return m_Swapchain;
	}

	VkRenderPass Swapchain::getRenderPass() const noexcept {
		return m_RenderPass;
	}

	VkExtent2D Swapchain::getExtent() const noexcept {
		return m_SwapChainExtent;
	}

	VkFormat Swapchain::getImageFormat() const noexcept {
		return m_SwapChainImageFormat;
	}

	const std::vector<VkImage>& Swapchain::getImages() const noexcept {
		return m_SwapChainImages;
	}

	const std::vector<OwningWrapper<VkImageView>>& Swapchain::getImageViews() const noexcept {
		return m_SwapChainImageViews;
	}

	const std::vector<OwningWrapper<VkSemaphore>>& Swapchain::getImageRenderFinishedSemaphores() const noexcept {
		return m_ImageRenderFinishedSemaphores;
	}
}