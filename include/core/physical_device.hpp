#pragma once
#ifndef MARCHING_CUBES_CORE_PHYSICAL_DEVICE_HPP
#define MARCHING_CUBES_CORE_PHYSICAL_DEVICE_HPP

#include <vulkan/vulkan_core.h>

#include <compare>
#include <mutex>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <core/wrapper.hpp>

namespace marching_cubes::core {

	class PhysicalDevice final : public WrapperTraits<PhysicalDevice, VkPhysicalDevice> {
	public:

		PhysicalDevice() noexcept = default;

		explicit PhysicalDevice(VkPhysicalDevice physicalDevice);

		PhysicalDevice(const PhysicalDevice&) = delete;
		PhysicalDevice& operator=(const PhysicalDevice&) = delete;

		PhysicalDevice(PhysicalDevice&&) noexcept = default;
		PhysicalDevice& operator=(PhysicalDevice&&) noexcept = default;

		~PhysicalDevice() = default;

		[[nodiscard]] VkPhysicalDevice getWrapped() const noexcept;

		[[nodiscard]] static const VkPhysicalDeviceProperties& getProperties(
			VkPhysicalDevice physicalDevice
		);

		[[nodiscard]] static const VkFormatProperties& getFormatProperties(
			VkPhysicalDevice physicalDevice,
			VkFormat format
		);

		[[nodiscard]] static const VkPhysicalDeviceFeatures& getFeatures(
			VkPhysicalDevice physicalDevice
		);

		[[nodiscard]] static VkFormat findSupportedFormat(
			VkPhysicalDevice physicalDevice,
			const std::vector<VkFormat>& candidates,
			VkImageTiling tiling,
			VkFormatFeatureFlags features
		);

		[[nodiscard]] static VkFormat findDepthFormat(
			VkPhysicalDevice physicalDevice
		);

		[[nodiscard]] static VkSampleCountFlagBits getMaxUsableSampleCount(
			VkPhysicalDevice physicalDevice
		);

	private:

		static inline std::unordered_map<
			VkPhysicalDevice,
			VkPhysicalDeviceProperties
		> s_PropertiesCache;
		static inline std::mutex s_PropertiesMutex;

		static inline std::unordered_map<
			VkPhysicalDevice,
			std::unordered_map<VkFormat, VkFormatProperties>
		> s_FormatPropertiesCache;
		static inline std::mutex s_FormatPropertiesMutex;

		static inline std::unordered_map<
			VkPhysicalDevice,
			VkPhysicalDeviceFeatures
		> s_FeaturesCache;
		static inline std::mutex s_FeaturesMutex;

		VkPhysicalDevice m_PhysicalDevice{};
	};
}

#endif // !MARCHING_CUBES_CORE_PHYSICAL_DEVICE_HPP

