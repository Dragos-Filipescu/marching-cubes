#include <core/physical_device.hpp>

namespace marching_cubes::core {

    PhysicalDevice::PhysicalDevice(VkPhysicalDevice physicalDevice)
        : m_PhysicalDevice{ physicalDevice }
    {
    }

    VkPhysicalDevice PhysicalDevice::getWrapped() const noexcept
    {
        return m_PhysicalDevice;
    }

    const VkPhysicalDeviceProperties& PhysicalDevice::getProperties(
        VkPhysicalDevice physicalDevice
    )
    {
        std::lock_guard lock{ s_PropertiesMutex };
        if (!s_PropertiesCache.contains(physicalDevice)) {
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(physicalDevice, &properties);
            s_PropertiesCache.try_emplace(physicalDevice, properties);
        }
        return s_PropertiesCache[physicalDevice];
    }

    const VkFormatProperties& PhysicalDevice::getFormatProperties(
        VkPhysicalDevice physicalDevice,
        VkFormat format
    )
    {
        std::lock_guard lock{ s_FormatPropertiesMutex };
        if (!s_FormatPropertiesCache.contains(physicalDevice)) {
            s_FormatPropertiesCache.try_emplace(physicalDevice);
        }
        auto& innerMap = s_FormatPropertiesCache[physicalDevice];
        if (!innerMap.contains(format)) {
            VkFormatProperties formatProperties{};
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
            innerMap.try_emplace(format, formatProperties);
        }
        return innerMap[format];
    }

    const VkPhysicalDeviceFeatures& PhysicalDevice::getFeatures(
        VkPhysicalDevice physicalDevice
    )
    {
        std::lock_guard lock{ s_FeaturesMutex };
        if (!s_FeaturesCache.contains(physicalDevice)) {
            VkPhysicalDeviceFeatures features{};
            vkGetPhysicalDeviceFeatures(physicalDevice, &features);
            s_FeaturesCache.try_emplace(physicalDevice, features);
        }
        return s_FeaturesCache[physicalDevice];
    }

    VkFormat PhysicalDevice::findSupportedFormat(
        VkPhysicalDevice physicalDevice,
        const std::vector<VkFormat>& candidates,
        VkImageTiling tiling,
        VkFormatFeatureFlags features
    )
    {
        for (VkFormat format : candidates) {
            const auto& props = getFormatProperties(physicalDevice, format);
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        throw std::runtime_error("failed to find supported format!");
    }

    VkFormat PhysicalDevice::findDepthFormat(VkPhysicalDevice physicalDevice)
    {
        return findSupportedFormat(
            physicalDevice,
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    VkSampleCountFlagBits PhysicalDevice::getMaxUsableSampleCount(
        VkPhysicalDevice physicalDevice
    )
    {
        const auto& physicalDeviceProperties = getProperties(physicalDevice);
        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts
            & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

        if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
        if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
        if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
        if (counts & VK_SAMPLE_COUNT_8_BIT)  return VK_SAMPLE_COUNT_8_BIT;
        if (counts & VK_SAMPLE_COUNT_4_BIT)  return VK_SAMPLE_COUNT_4_BIT;
        if (counts & VK_SAMPLE_COUNT_2_BIT)  return VK_SAMPLE_COUNT_2_BIT;

        return VK_SAMPLE_COUNT_1_BIT;
    }
}
