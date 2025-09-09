#pragma once
#ifndef MARCHING_CUBES_CORE_IMAGE_VIEW_HPP
#define MARCHING_CUBES_CORE_IMAGE_VIEW_HPP

#include <vulkan/vulkan_core.h>

#include <core/wrapper.hpp>

namespace marching_cubes::core {
	class ImageView final : public WrapperTraits<ImageView, VkImageView> {
	public:

		ImageView() noexcept = default;

		ImageView(
			VkDevice device,
			VkImageViewCreateInfo createInfo
		);

		ImageView(const ImageView&) = delete;
		ImageView& operator=(const ImageView&) = delete;

		ImageView(ImageView&&) noexcept = default;
		ImageView& operator=(ImageView&&) noexcept = default;

		~ImageView() = default;

		[[nodiscard]] VkImageView getWrapped() const noexcept;
		[[nodiscard]] const VkImageViewCreateInfo& getCreateInfo() const noexcept;

	private:
		// owned
		OwningWrapper<VkImageView>	m_ImageView{};
		VkImageViewCreateInfo		m_CreateInfo{};
	};
}

#endif // !MARCHING_CUBES_CORE_IMAGE_VIEW_HPP

