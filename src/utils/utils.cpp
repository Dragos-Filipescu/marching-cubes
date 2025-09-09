#include <utils/utils.hpp>

#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cstring>
#include <format>
#include <fstream>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <core/aliases.hpp>
#include <type_traits>
#include <cmath>

namespace marching_cubes {

    std::vector<char> readFile(
        const std::string& filePath
    )
    {
        std::ifstream file(filePath, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error{ "Failed to open file!" };
        }

        std::size_t fileSize = static_cast<std::size_t>(file.tellg());
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }

    bool checkValidationLayerSupport(
        const std::vector<const char*>& validationLayers
    ) noexcept
    {
        u32 layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const auto& layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (std::strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    std::vector<const char*> getRequiredExtensions(
        bool enableValidationLayers
    ) noexcept
    {
        u32 glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions{ glfwExtensions, glfwExtensions + glfwExtensionCount };

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& availableFormats
    ) noexcept
    {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& availablePresentModes
    ) noexcept
    {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(
        GLFWwindow* window,
        VkSurfaceCapabilitiesKHR capabilities
    ) noexcept
    {
        if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
            return capabilities.currentExtent;
        }
        else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<u32>(width),
                static_cast<u32>(height)
            };

            std::cout << "Choosing swap extent: width=" << actualExtent.width
                << " height=" << actualExtent.height << '\n';

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    bool checkDeviceExtensionSupport(
        VkPhysicalDevice physicalDevice
    ) noexcept
    {
        u32 extensionCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string_view> requiredExtensions(c_DeviceExtensions.begin(), c_DeviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    SwapChainSupportDetails querySwapChainSupport(
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR surface
    ) noexcept
    {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

        u32 formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
        }

        u32 presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    QueueFamilyIndices findQueueFamilies(
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR surface
    ) noexcept
    {
        QueueFamilyIndices indices;

        u32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
        u32 i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }
        return indices;
    }

    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const char* pName,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger
    ) noexcept
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, pName);
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator
    ) noexcept
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    void populateDebugMessengerCreateInfo(
        VkDebugUtilsMessengerCreateInfoEXT& createInfo,
        PFN_vkDebugUtilsMessengerCallbackEXT callback
    ) noexcept
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = callback;
    }

    bool isDeviceSuitable(
        VkPhysicalDevice device,
        VkSurfaceKHR surface
    ) noexcept
    {
        const auto& indices = findQueueFamilies(device, surface);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            const auto& swapChainSupport = querySwapChainSupport(device, surface);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    void* copyDataToDevice(
        VkDevice device,
        VkDeviceMemory memory,
        const void* data,
        VkDeviceSize size,
        VkDeviceSize offset,
        VkMemoryMapFlags flags,
        bool unmapMemory
    )
    {
		void* mappedData;
        if (vkMapMemory(
            device,
            memory,
            offset,
            size,
            flags,
            &mappedData
        ) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to map memory!" };
        }
        std::memcpy(mappedData, data, static_cast<std::size_t>(size));
        if (unmapMemory) {
            vkUnmapMemory(device, memory);
        }
        return mappedData;
	}

    VkDeviceMemory bindBufferMemory(
        VkDevice device,
        VkBuffer buffer,
        VkDeviceMemory memory,
        VkDeviceSize memoryOffset
    )
    {
        if (vkBindBufferMemory(
            device,
            buffer,
            memory,
            memoryOffset
        ) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to bind buffer memory!" };
        }
        return memory;
    }

    VkDeviceMemory bindImageMemory(
        VkDevice device,
        VkImage image,
        VkDeviceMemory memory,
        VkDeviceSize memoryOffset
    )
    {
        if (vkBindImageMemory(
            device,
            image,
            memory,
            memoryOffset
        ) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to bind image memory!" };
        }
        return memory;
    }

    std::size_t vkFormatToSizeBytes(VkFormat format)
    {
        switch (format) {
            // 8-bit formats
        case VK_FORMAT_R8_UNORM:
            [[fallthrough]];
        case VK_FORMAT_R8_SNORM:
            [[fallthrough]];
        case VK_FORMAT_R8_UINT:
            [[fallthrough]];
        case VK_FORMAT_R8_SINT:
            [[fallthrough]];
        case VK_FORMAT_R8_SRGB:
            return 1;

            // 16-bit formats
        case VK_FORMAT_R16_UNORM:
            [[fallthrough]];
        case VK_FORMAT_R16_SNORM:
            [[fallthrough]];
        case VK_FORMAT_R16_UINT:
            [[fallthrough]];
        case VK_FORMAT_R16_SINT:
            [[fallthrough]];
        case VK_FORMAT_R16_SFLOAT:
            [[fallthrough]];
        case VK_FORMAT_R8G8_UNORM:
            [[fallthrough]];
        case VK_FORMAT_R8G8_SNORM:
            [[fallthrough]];
        case VK_FORMAT_R8G8_UINT:
            [[fallthrough]];
        case VK_FORMAT_R8G8_SINT:
            [[fallthrough]];
        case VK_FORMAT_R8G8_SRGB:
            return 2;

            // 24-bit not common – usually packed, so omitted

            // 32-bit formats
        case VK_FORMAT_R32_UINT:
            [[fallthrough]];
        case VK_FORMAT_R32_SINT:
            [[fallthrough]];
        case VK_FORMAT_R32_SFLOAT:
            [[fallthrough]];
        case VK_FORMAT_R16G16_UNORM:
            [[fallthrough]];
        case VK_FORMAT_R16G16_SFLOAT:
            [[fallthrough]];
        case VK_FORMAT_R8G8B8A8_UNORM:
            [[fallthrough]];
        case VK_FORMAT_R8G8B8A8_SNORM:
            [[fallthrough]];
        case VK_FORMAT_R8G8B8A8_UINT:
            [[fallthrough]];
        case VK_FORMAT_R8G8B8A8_SINT:
            [[fallthrough]];
        case VK_FORMAT_R8G8B8A8_SRGB:
            [[fallthrough]];
        case VK_FORMAT_B8G8R8A8_UNORM:
            [[fallthrough]];
        case VK_FORMAT_B8G8R8A8_SRGB:
            return 4;

            // 64-bit formats
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            [[fallthrough]];
        case VK_FORMAT_R16G16B16A16_UNORM:
            [[fallthrough]];
        case VK_FORMAT_R32G32_SFLOAT:
            [[fallthrough]];
        case VK_FORMAT_R32G32_UINT:
            [[fallthrough]];
        case VK_FORMAT_R32G32_SINT:
            return 8;

            // 128-bit formats
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            [[fallthrough]];
        case VK_FORMAT_R32G32B32A32_UINT:
            [[fallthrough]];
        case VK_FORMAT_R32G32B32A32_SINT:
            return 16;

        default:
            throw std::runtime_error{ "Unsupported or compressed VkFormat in vkFormatToSizeBytes" };
        }
    }

    int vkFormatToStbiChannels(VkFormat format) noexcept
    {
        switch (format)
        {
            //------------------------------------
            // 8-bit UNORM / SRGB formats
            //------------------------------------

            // 1 channel
        case VK_FORMAT_R8_UNORM:
            [[fallthrough]];
        case VK_FORMAT_R8_SRGB:
            return STBI_grey;

            // 2 channels
        case VK_FORMAT_R8G8_UNORM:
            [[fallthrough]];
        case VK_FORMAT_R8G8_SRGB:   // rarely used but exists
            return STBI_grey_alpha;

            // 3 channels
        case VK_FORMAT_R8G8B8_UNORM:
            [[fallthrough]];
        case VK_FORMAT_R8G8B8_SRGB:
            return STBI_rgb;

        case VK_FORMAT_B8G8R8_UNORM:
            [[fallthrough]];
        case VK_FORMAT_B8G8R8_SRGB:
            return STBI_rgb;

            // 4 channels
        case VK_FORMAT_R8G8B8A8_UNORM:
            [[fallthrough]];
        case VK_FORMAT_R8G8B8A8_SRGB:
            return STBI_rgb_alpha;

        case VK_FORMAT_B8G8R8A8_UNORM:
            [[fallthrough]];
        case VK_FORMAT_B8G8R8A8_SRGB:
            return STBI_rgb_alpha;

            //------------------------------------
            // Rare packed formats (basically ignore)
            //------------------------------------

            // You could technically map some of these packed formats but stb_image
            // doesn't support them natively, so better just reject:
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
            [[fallthrough]];
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            [[fallthrough]];
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
            [[fallthrough]];
        case VK_FORMAT_B5G6R5_UNORM_PACK16:
            [[fallthrough]];
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
            [[fallthrough]];
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
            [[fallthrough]];
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
            [[fallthrough]];
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
            return STBI_default;

            //------------------------------------
            // Formats STBI can't load:
            //------------------------------------

            // 16-bit formats
        case VK_FORMAT_R16_UNORM:
            [[fallthrough]];
        case VK_FORMAT_R16G16_UNORM:
            [[fallthrough]];
        case VK_FORMAT_R16G16B16_UNORM:
            [[fallthrough]];
        case VK_FORMAT_R16G16B16A16_UNORM:
            return STBI_default;

            // Float formats
        case VK_FORMAT_R16_SFLOAT:
            [[fallthrough]];
        case VK_FORMAT_R16G16_SFLOAT:
            [[fallthrough]];
        case VK_FORMAT_R16G16B16_SFLOAT:
            [[fallthrough]];
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            [[fallthrough]];
        case VK_FORMAT_R32_SFLOAT:
            [[fallthrough]];
        case VK_FORMAT_R32G32_SFLOAT:
            [[fallthrough]];
        case VK_FORMAT_R32G32B32_SFLOAT:
            [[fallthrough]];
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return STBI_default;

            // Depth-stencil formats
        case VK_FORMAT_D16_UNORM:
            [[fallthrough]];
        case VK_FORMAT_D32_SFLOAT:
            [[fallthrough]];
        case VK_FORMAT_D24_UNORM_S8_UINT:
            [[fallthrough]];
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            [[fallthrough]];
        case VK_FORMAT_S8_UINT:
            return STBI_default;

            // Compressed formats (BCn, ETC, ASTC, PVRTC, etc)
            // -> totally unsupported by STBI natively
        default:
            return STBI_default;
        }
    }

    bool hasStencilComponent(VkFormat format) noexcept
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    LayoutTransitionBarrier prepareImageLayoutTransition(
        VkImage image,
        const VkImageCreateInfo& imageInfo,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        VkImageSubresourceRange subresourceRange
    )
    {
        LayoutTransitionBarrier result{};
        result.barrier = VkImageMemoryBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = 0, // Set below
            .dstAccessMask = 0, // Set below
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = subresourceRange,
        };

        // Default aspect mask based on format
        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            result.barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (hasStencilComponent(imageInfo.format)) {
                result.barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }
        else {
            result.barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        /////////////////////////////////
        // BEG Colored image transfers //
        /////////////////////////////////
        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            // Inital transfer for colored image. Change layout to TRANSFER_DST_OPTIMAL.
            result.barrier.srcAccessMask = 0; // Undefined contents, we do not need to wait on anything.
            result.barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // We are going to perform a transfer.
            result.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // We are not waiting for anything earlier.
            result.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT; // We are going to perform a transfer.
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            // Modify image to sample from shader. Change layout to SHADER_READ_ONLY_OPTIMAL.
            result.barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // We must wait for prior transfer.
            result.barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // We are going to read this from a shader.
            result.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT; // We are waiting on a prior transfer.
            result.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // We are going to read this from the frag shader.
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            // Modify image to transfer contents again (usually for overwrites). Change layout to TRANSFER_DST_OPTIMAL.
            result.barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT; // We want to ensure all prior reads are complete before transitioning.
            result.barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // We are going to perform a transfer.
            result.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // We could still be reading from the frag shader.
            result.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT; // We are going to perform a transfer.
        }
        /////////////////////////////////
        // END Colored image transfers //
        /////////////////////////////////

        //////////////////
        // BEG Blitting //
        //////////////////
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
            // Modify image layout for blitting, from initial undefined contents (read). Change layout to SRC_OPTIMAL;
            result.barrier.srcAccessMask = 0; // Undefined contents, we do not need to wait on anything.
            result.barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT; // We are going to perform a read transfer.
            result.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // We are not waiting for anything earlier.
            result.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT; // We are going to perform a transfer.
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            // Modify image layout for blitting, from initial undefined contents (write). Change layout to DST_OPTIMAL;
            result.barrier.srcAccessMask = 0; // Undefined contents, we do not need to wait on anything.
            result.barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // We are going to perform a write transfer.
            result.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // We are not waiting for anything earlier.
            result.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT; // We are going to perform a transfer.
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
            // Modify image layout for blitting (read). Change layout to SRC_OPTIMAL;
            result.barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // We must wait for prior write transfer.
            result.barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT; // We are going to perform a read transfer.
            result.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT; // We are waiting on a prior transfer.
            result.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT; // We are going to perform a transfer.
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            // Modify image layout for blitting (write). Change layout to DST_OPTIMAL;
            result.barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT; // We must wait for prior read transfer.
            result.barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // We are going to perform a write transfer.
            result.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT; // We are waiting on a prior transfer.
            result.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT; // We are going to perform a transfer.
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            // Modify image from write destination to read source. Change layout to TRANSFER_SRC_OPTIMAL.
            result.barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // We must wait for prior transfer.
            result.barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // We are going to perform a transfer.
            result.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT; // We are waiting on a prior transfer.
            result.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // We are going to read this from the frag shader.
        }
        //////////////////
        // END Blitting //
        //////////////////


        ///////////////////////////////
        // BEG Depth image transfers //
        ///////////////////////////////
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            // Inital transfer for depth image. Change layout to DEPTH_STENCIL_ATTACHMENT_OPTIMAL.
            result.barrier.srcAccessMask = 0; // Undefined contents, we do not need to wait on anything.
            result.barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; // We will both read from, and write to, the depth stencil.
            result.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // We are not waiting for anything earlier.
            result.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // Depth/Stencil tests happen early in the pipeline (as opposed to late via VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT).
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            // Modify image to sample from shader. Change layout to SHADER_READ_ONLY_OPTIMAL.
            result.barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; // We wait for stencil tests to finish before sampling.
            result.barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // We are going to read this from a shader.
            result.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT; // We wait for the latest stage of depth writes.
            result.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // We are going to read this from the frag shader.
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            // Modify image to transfer contents again (usually for overwrites). Change layout to DEPTH_STENCIL_ATTACHMENT_OPTIMAL.
            result.barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT; // We want to ensure all prior reads are complete before transitioning.
            result.barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; // We are going to write depths again.
            result.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // We could still be reading from the frag shader.
            result.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // Depth/Stencil tests happen early in the pipeline (as opposed to late via VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT).
        }
        ///////////////////////////////
        // END Depth image transfers //
        ///////////////////////////////

        else {
            throw std::invalid_argument{
                std::format(
                    "unsupported layout transition: {} -> {}!",
                    static_cast<std::underlying_type_t<decltype(oldLayout)>>(oldLayout),
                    static_cast<std::underlying_type_t<decltype(newLayout)>>(newLayout)
                )
            };
        }

        return result;
    }

    ImageData getImageData(
        const std::string& path,
        VkFormat format,
        bool mipMapping
    )
    {
        int width, height, channels;
        const stbi_uc* pixels = stbi_load(path.data(), &width, &height, &channels, vkFormatToStbiChannels(format));

        if (!pixels) {
            throw std::runtime_error{ "Failed to load texture image!" };
        }

        u32 mipLevels = static_cast<u32>(std::floor(std::log2(std::max(width, height)))) + 1;

        return ImageData{
            VkExtent3D{
                .width = static_cast<u32>(width),
                .height = static_cast<u32>(height),
                .depth = 1,
            },
            mipMapping ? mipLevels : 1u,
            std::vector<u8>(
                pixels,
                pixels + vkFormatToSizeBytes(format) * static_cast<std::size_t>(width) * static_cast<std::size_t>(height)
            ),
        };
    }
}
