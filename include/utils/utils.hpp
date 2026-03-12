#pragma once
#ifndef MARCHING_CUBES_UTILS_UTILS_HPP
#define MARCHING_CUBES_UTILS_UTILS_HPP

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/detail/type_quat.hpp>
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <compare>
#include <concepts>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <core/aliases.hpp>

namespace marching_cubes {

#ifdef NDEBUG
    constexpr bool kEnableValidationLayers = true;
#else
    constexpr bool kEnableValidationLayers = true;
#endif

    inline std::ostream& operator<<(std::ostream& out, const glm::ivec2& vec)
    {
        out << "[" << vec.x << " " << vec.y << "]";
        return out;
    }

    inline std::ostream& operator<<(std::ostream& out, const glm::vec2& vec)
    {
        out << "[" << vec.x << " " << vec.y << "]";
        return out;
    }

    inline std::ostream& operator<<(std::ostream& out, const glm::ivec3& vec)
    {
        out << "[" << vec.x << " " << vec.y << " " << vec.z << "]";
        return out;
    }

    inline std::ostream& operator<<(std::ostream& out, const glm::vec3& vec)
    {
        out << "[" << vec.x << " " << vec.y << " " << vec.z << "]";
        return out;
    }

    inline std::ostream& operator<<(std::ostream& out, const glm::ivec4& vec)
    {
        out << "[" << vec.x << " " << vec.y << " " << vec.z << " " << vec.w << "]";
        return out;
    }

    inline std::ostream& operator<<(std::ostream& out, const glm::vec4& vec)
    {
        out << "[" << vec.x << " " << vec.y << " " << vec.z << " " << vec.w << "]";
        return out;
    }

    inline std::ostream& operator<<(std::ostream& out, const glm::quat& rot)
    {
        out << "[" << rot.x << " " << rot.y << " " << rot.z << " " << rot.w << "]";
        return out;
    }

    [[nodiscard]] constexpr bool operator==(const VkExtent2D& lhs, const VkExtent2D& rhs) noexcept
    {
        return lhs.width == rhs.width
            && lhs.height == rhs.height;
    }

    [[nodiscard]] constexpr bool operator==(const VkExtent3D& lhs, const VkExtent3D& rhs) noexcept
    {
        return lhs.width == rhs.width
            && lhs.height == rhs.height
            && lhs.depth == rhs.depth;
    }

    [[nodiscard]] constexpr bool operator==(const VkSubresourceLayout& lhs, const VkSubresourceLayout& rhs) noexcept
    {
        return lhs.offset == rhs.size
            && lhs.size == rhs.size
            && lhs.rowPitch == rhs.rowPitch
            && lhs.arrayPitch == rhs.arrayPitch
            && lhs.depthPitch == rhs.depthPitch;
    }

    [[nodiscard]] constexpr bool operator==(const VkImageCreateInfo& lhs, const VkImageCreateInfo& rhs) noexcept
    {

        const bool queueFamiliesEqual = lhs.queueFamilyIndexCount == rhs.queueFamilyIndexCount
            && (lhs.queueFamilyIndexCount == 0
                || (lhs.pQueueFamilyIndices != nullptr
                    && rhs.pQueueFamilyIndices != nullptr
                    && std::equal(
                        lhs.pQueueFamilyIndices,
                        lhs.pQueueFamilyIndices + lhs.queueFamilyIndexCount,
                        rhs.pQueueFamilyIndices,
                        rhs.pQueueFamilyIndices + rhs.queueFamilyIndexCount
                    ))
                );

        return lhs.sType == rhs.sType
            && lhs.flags == rhs.flags
            && lhs.pNext == rhs.pNext
            && lhs.imageType == rhs.imageType
            && lhs.format == rhs.format
            && lhs.extent == rhs.extent
            && lhs.mipLevels == rhs.mipLevels
            && lhs.arrayLayers == rhs.arrayLayers
            && lhs.samples == rhs.samples
            && lhs.tiling == rhs.tiling
            && lhs.usage == rhs.usage
            && lhs.sharingMode == rhs.sharingMode
            && queueFamiliesEqual
            && lhs.initialLayout == rhs.initialLayout;
    }

    template<std::size_t N, typename T, glm::qualifier Q>
    glm::vec<N, T, Q> vec_nextafter(
        const glm::vec<N, T, Q>& x,
        const glm::vec<N, T, Q>& y
    ) noexcept
    {
        glm::vec<N, T, Q> out;
        for (std::size_t i = 0; i < N; ++i) {
            out[i] = std::nextafter(x[i], y[i]);
        }
        return out;
    }

    template<
        typename R,
        std::floating_point T,
        std::floating_point U
    >
    [[nodiscard]] R floor_div(T value, U div) noexcept
    {
        return static_cast<R>(std::floor(value / div));
    }

    template<
        typename R,
        std::floating_point T,
        std::floating_point U,
        std::size_t N,
        glm::qualifier Q = glm::packed_highp
    >
    [[nodiscard]] glm::vec<N, R, Q> floor_div(
        const glm::vec<N, T, Q>& value,
        U div
    )
    {
		return glm::vec<N, R, Q>{ glm::floor(value / div) };
    }

    template<
        typename R,
        std::floating_point T,
        std::floating_point U
    >
    [[nodiscard]] R ceil_div(T value, U div) noexcept
    {
        return static_cast<R>(std::ceil(value / div));
    }

    template<
        typename R,
        std::floating_point T,
        std::floating_point U,
        std::size_t N,
        glm::qualifier Q = glm::packed_highp
    >
    [[nodiscard]] glm::vec<N, R, Q> ceil_div(
        const glm::vec<N, T, Q>& value,
        U div
    )
    {
        return glm::vec<N, R, Q>{ glm::ceil(value / div) };
    }

    template<
        typename R,
        std::floating_point T,
        std::floating_point U
    >
    [[nodiscard]] R bumped_floor_div(T value, U div) noexcept
    {
        using CT = std::common_type_t<T, U>;
        const CT q = static_cast<CT>(value) / static_cast<CT>(div);
        const CT bumped = std::nextafter(q, std::numeric_limits<CT>::infinity());
        return static_cast<R>(std::floor(bumped));
    }

    template<
        typename R,
        std::floating_point T,
        std::floating_point U
    >
    [[nodiscard]] R bumped_ceil_div(T value, U div) noexcept
    {
        using CT = std::common_type_t<T, U>;
        const CT q = static_cast<CT>(value) / static_cast<CT>(div);
        const CT bumped = std::nextafter(q, -std::numeric_limits<CT>::infinity());
        return static_cast<R>(std::ceil(bumped));
    }

    template<
        typename R,
        std::floating_point T,
        std::floating_point U,
        std::size_t N,
        glm::qualifier Q = glm::packed_highp
    >
    [[nodiscard]] glm::vec<N, R, Q> bumped_floor_div(
        const glm::vec<N, T, Q>& value,
        U div,
        std::common_type_t<T, U> dir = std::numeric_limits<std::common_type_t<T, U>>::infinity()
    ) noexcept
    {
        using CT = std::common_type_t<T, U>;
        glm::vec<N, CT, Q> q = glm::vec<N, CT, Q>(value) / static_cast<CT>(div);

        glm::vec<N, CT, Q> direction(dir);
        glm::vec<N, CT, Q> bumped = vec_nextafter(q, direction);

        return glm::vec<N, R, Q>(glm::floor(bumped));
    }

    template<
        typename R,
        std::floating_point T,
        std::floating_point U,
        std::size_t N,
        glm::qualifier Q = glm::packed_highp
    >
    [[nodiscard]] glm::vec<N, R, Q> bumped_ceil_div(
        const glm::vec<N, T, Q>& value,
        U div,
        std::common_type_t<T, U> dir = -std::numeric_limits<std::common_type_t<T, U>>::infinity()
    ) noexcept
    {
        using CT = std::common_type_t<T, U>;
        glm::vec<N, CT, Q> q = glm::vec<N, CT, Q>(value) / static_cast<CT>(div);

        glm::vec<N, CT, Q> direction(dir);
        glm::vec<N, CT, Q> bumped = vec_nextafter(q, direction);

        return glm::vec<N, R, Q>(glm::ceil(bumped));
    }

    template<typename T>
    [[nodiscard]] constexpr T volume(const glm::vec<3, T>& extent) noexcept
    {
        return extent.x * extent.y * extent.z;
	}

    template<std::integral T>
    [[nodiscard]] constexpr T linearize(
        const glm::vec<3, T>& extent,
        const glm::vec<3, std::size_t>& sizes
    ) noexcept
    {
        return static_cast<std::size_t>(extent.x)
            + sizes.x * (
                static_cast<std::size_t>(extent.y)
                + sizes.y * static_cast<std::size_t>(extent.z)
            );
    }

    template <typename T>
    [[nodiscard]] constexpr std::size_t hash_combine(std::size_t seed, const T& v) noexcept
    {
        static constexpr std::size_t GOLDEN_RATIO =
            sizeof(std::size_t) == 8
            ? 0x9e3779b97f4a7c15ULL
            : 0x9e3779b9ULL;
        std::hash<T> hasher{};
        return seed ^ hasher(v) + GOLDEN_RATIO + (seed << 6) + (seed >> 2);
    }

    struct VkImageSubresourceHasher final {
        [[nodiscard]] constexpr std::size_t operator()(const VkImageSubresource& subresource) const noexcept
        {
            std::size_t seed = 0;
            seed = hash_combine(seed, subresource.aspectMask);
            seed = hash_combine(seed, subresource.mipLevel);
            seed = hash_combine(seed, subresource.arrayLayer);
            return seed;
        }
    };

    static constinit std::array<const char*, 1> c_ValidationLayers {
        "VK_LAYER_KHRONOS_validation"
    };

    static constinit std::array<const char*, 1> c_DeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    constexpr glm::vec3 kVec3Up{ 0.0f, 1.0f, 0.0f };
    constexpr glm::vec3 kVec3Down{ 0.0f, -1.0f, 0.0f };
    constexpr glm::vec3 kVec3Left{ -1.0f, 0.0f, 0.0f };
    constexpr glm::vec3 kVec3Right{ 1.0f, 0.0f, 0.0f };
    constexpr glm::vec3 kVec3Forward{ 0.0f, 0.0f, -1.0f };
    constexpr glm::vec3 kVec3Backward{ 0.0f, 0.0f, 1.0f };

    template<std::size_t Alignment>
        requires (Alignment > 0)
    constexpr std::size_t alignUp(std::size_t size) noexcept {
        if constexpr ((Alignment & (Alignment - 1)) == 0) {
            // fast path for power-of-two alignments
            return ((size + (Alignment - 1)) & ~(Alignment - 1));
        }
        return ((size + Alignment - 1) / Alignment) * Alignment;
    }

    struct ImageData final {
        VkExtent3D extent{};
        u32 mipLevels{};
        std::vector<u8> data{};

        ImageData() = default;

        ImageData(
            VkExtent3D extent,
            u32 mipLevels,
            std::vector<u8>&& data
        )
            : extent{ extent },
            mipLevels{ mipLevels },
            data{ std::move(data) }
        {
        }

        ImageData(
            VkExtent3D extent,
            u32 mipLevels,
            const std::vector<u8>& data
        )
            : extent{ extent },
            mipLevels{ mipLevels },
            data{ data }
        {
        }
    };

    struct SwapChainSupportDetails final {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats{};
        std::vector<VkPresentModeKHR> presentModes{};
    };

    struct QueueFamilyIndices final {
        std::optional<u32> graphicsFamily{};
        std::optional<u32> presentFamily{};

        [[nodiscard]] constexpr bool isComplete() const noexcept {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct LayoutTransitionBarrier {
        VkImageMemoryBarrier barrier{};
        VkPipelineStageFlags srcStageMask{};
        VkPipelineStageFlags dstStageMask{};
    };

    [[nodiscard]] std::vector<char> readFile(
        const std::string& filePath
    );

    [[nodiscard]] bool checkValidationLayerSupport(
        const std::vector<const char*>& validationLayers
    ) noexcept;

    [[nodiscard]] std::vector<const char*> getRequiredExtensions(
        bool enableValidationLayers
    ) noexcept;

    [[nodiscard]] VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& availableFormats
    ) noexcept;

    [[nodiscard]] VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& availablePresentModes
    ) noexcept;

    [[nodiscard]] VkExtent2D chooseSwapExtent(
        GLFWwindow* window,
        VkSurfaceCapabilitiesKHR capabilities
    ) noexcept;

    [[nodiscard]] bool checkDeviceExtensionSupport(
        VkPhysicalDevice physicalDevice
    ) noexcept;

    [[nodiscard]] SwapChainSupportDetails querySwapChainSupport(
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR surface
    ) noexcept;

    [[nodiscard]] QueueFamilyIndices findQueueFamilies(
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR surface
    ) noexcept;

    [[nodiscard]] VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const char* pName,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger
    ) noexcept;

    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator
    ) noexcept;

    void populateDebugMessengerCreateInfo(
        VkDebugUtilsMessengerCreateInfoEXT& createInfo,
        PFN_vkDebugUtilsMessengerCallbackEXT callback
    ) noexcept;

    [[nodiscard]] bool isDeviceSuitable(
        VkPhysicalDevice device,
        VkSurfaceKHR surface
    ) noexcept;

    void* copyDataToDevice(
        VkDevice device,
        VkDeviceMemory memory,
        const void* data,
        VkDeviceSize size,
        VkDeviceSize offset = 0,
        VkMemoryMapFlags flags = 0,
		bool unmapMemory = true
    );

    VkDeviceMemory bindBufferMemory(
        VkDevice device,
        VkBuffer buffer,
        VkDeviceMemory memory,
        VkDeviceSize memoryOffset = 0
    );

    VkDeviceMemory bindImageMemory(
        VkDevice device,
        VkImage image,
        VkDeviceMemory memory,
        VkDeviceSize memoryOffset = 0
    );

    [[nodiscard]] std::size_t vkFormatToSizeBytes(VkFormat format);

    [[nodiscard]] int vkFormatToStbiChannels(VkFormat format) noexcept;

    template<typename... Args>
        requires (... && std::convertible_to<Args, std::string_view>)
    [[nodiscard]] inline std::string joinPaths(std::string_view first, Args&&... rest) {
        std::string result(first);
        auto append_part = [](std::string& lhs, std::string_view rhs) {
            if (!lhs.empty() && lhs.back() != '/') {
                lhs.push_back('/');
            }
            lhs.append(rhs);
        };

        (append_part(result, rest), ...);
        return result;
    }

    [[nodiscard]] bool hasStencilComponent(VkFormat format) noexcept;

    [[nodiscard]] LayoutTransitionBarrier prepareImageLayoutTransition(
        VkImage image,
        const VkImageCreateInfo& imageInfo,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        VkImageSubresourceRange subresourceRange
    );

    [[nodiscard]] ImageData getImageData(
        const std::string& path,
        VkFormat format,
        bool mipMapping = true
    );
}

#endif // !MARCHING_CUBES_UTILS_UTILS_HPP
