#include <core/helpers/swapchain.hpp>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <ranges>

#include <core/helpers/image_view.hpp>
#include <core/helpers/semaphore.hpp>

namespace marching_cubes::core::helpers {

	std::vector<VkImage> getSwapChainImages(
		VkDevice device,
		VkSwapchainKHR swapchain
	) {
		std::uint32_t imageCount;
		vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
		std::vector<VkImage> swapChainImages(imageCount);
		vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapChainImages.data());
		return swapChainImages;
	}

	std::vector<OwningWrapper<VkImageView>> createImageViews(
		VkDevice device,
		const std::vector<VkImage>& swapchainImages,
		VkFormat format
	) {
		VkImageViewCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.image = VK_NULL_HANDLE,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = format,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};

		return swapchainImages
			| std::ranges::views::transform(
				[&](VkImage image) {
					createInfo.image = image;
					return OwningWrapper{
						createImageView(
							device,
							createInfo
						),
						deleters::VkImageViewDeleter{ device }
					};
				}
			)
			| std::ranges::to<std::vector>();
	}

	std::vector<OwningWrapper<VkSemaphore>> createRenderFinishedSemaphores(
		VkDevice device,
		std::size_t count
	) {
		std::vector<OwningWrapper<VkSemaphore>> semaphores{};
		semaphores.reserve(count);

		VkSemaphoreCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0
		};

		for (std::size_t i = 0; i < count; ++i) {
			semaphores.emplace_back(
				createSemaphore(
					device,
					createInfo
				),
				deleters::VkSemaphoreDeleter{ device }
			);
		}

		return semaphores;
	}
}
