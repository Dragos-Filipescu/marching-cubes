#pragma once
#ifndef MARCHING_CUBES_CORE_IMAGE_HPP
#define MARCHING_CUBES_CORE_IMAGE_HPP

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <unordered_map>

#include <core/wrapper.hpp>

namespace marching_cubes::core {

	class Image final : public WrapperTraits<Image, VkImage> {
	public:

		using LayoutsType = std::unordered_map<std::uint32_t, VkImageLayout>;
		using LayoutsConstReference = std::add_lvalue_reference_t<std::add_const_t<LayoutsType>>;

		Image() noexcept = default;

		Image(
			VkDevice device,
			VkImageCreateInfo imageCreateInfo
		);

		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;

		Image(Image&&) noexcept = default;
		Image& operator=(Image&&) noexcept = default;

		~Image() = default;

		[[nodiscard]] const VkImageCreateInfo& getCreateInfo() const noexcept;
		[[nodiscard]] LayoutsConstReference getCurrentLayouts() const noexcept;
		[[nodiscard]] VkImage getWrapped() const noexcept;
		[[nodiscard]] const VkMemoryRequirements& getMemoryRequirements() const noexcept;
		[[nodiscard]] VkDeviceSize getAllocationSize() const noexcept;
		[[nodiscard]] VkDeviceSize getAlignment() const noexcept;
		[[nodiscard]] VkDeviceSize getMemoryTypeBits() const noexcept;
		[[nodiscard]] VkDeviceSize alignUp(VkDeviceSize size) const noexcept;

		Image& setCurrentLayout(std::uint32_t mipLevel, VkImageLayout newLayout);
		Image& setCurrentLayouts(VkImageLayout newLayout);

	private:
		// owned
		VkImageCreateInfo		m_CreateInfo{};
		LayoutsType				m_CurrentLayouts{};
		OwningWrapper<VkImage>	m_Image{};
		VkMemoryRequirements	m_MemoryRequirements{};
	};
}

#endif // !MARCHING_CUBES_CORE_IMAGE_HPP

