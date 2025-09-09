#pragma once
#ifndef MARCHING_CUBES_CORE_DETAIL_DEFAULT_DELETERS_HPP
#define MARCHING_CUBES_CORE_DETAIL_DEFAULT_DELETERS_HPP

#include <vulkan/vulkan_core.h>

namespace marching_cubes::core::deleters {
	
	struct NoopDeleter final {

		constexpr NoopDeleter() = default;

		template<typename T>
		constexpr void operator()(T) const noexcept {}
	};

	struct VkBufferDeleter final {
		VkDevice device;
		const VkAllocationCallbacks* allocator;

		VkBufferDeleter() = default;

		VkBufferDeleter(
			VkDevice device,
			VkAllocationCallbacks* allocator = nullptr
		) noexcept
			: device{ device },
			allocator{ allocator }
		{
		}

		void operator()(VkBuffer buffer) const noexcept
		{
			vkDestroyBuffer(device, buffer, allocator);
		}
	};

	struct VkCommandPoolDeleter final {
		VkDevice device;
		const VkAllocationCallbacks* allocator;

		VkCommandPoolDeleter() = default;

		VkCommandPoolDeleter(
			VkDevice device,
			const VkAllocationCallbacks* allocator = nullptr
		) noexcept
			: device{ device },
			allocator{ allocator }
		{
		}

		void operator()(VkCommandPool commandPool) const noexcept
		{
			vkDestroyCommandPool(device, commandPool, allocator);
		}
	};

	struct VkDescriptorSetLayoutDeleter final {
		VkDevice device;
		const VkAllocationCallbacks* allocator;

		VkDescriptorSetLayoutDeleter() = default;

		VkDescriptorSetLayoutDeleter(
			VkDevice device,
			const VkAllocationCallbacks* allocator = nullptr
		) noexcept
			: device{ device },
			allocator{ allocator }
		{
		}

		void operator()(VkDescriptorSetLayout descriptorSetLayout) const noexcept
		{
			vkDestroyDescriptorSetLayout(device, descriptorSetLayout, allocator);
		}
	};

	struct VkDescriptorSetDeleter final {
		VkDevice device;
		VkDescriptorPool descriptorPool;

		VkDescriptorSetDeleter() = default;

		VkDescriptorSetDeleter(
			VkDevice device,
			VkDescriptorPool descriptorPool
		) noexcept
			: device{ device },
			descriptorPool{ descriptorPool }
		{
		}

		void operator()(VkDescriptorSet descriptorSet) const noexcept
		{
			vkFreeDescriptorSets(device, descriptorPool, 1, &descriptorSet);
		}
	};

	struct VkDescriptorPoolDeleter final {
		VkDevice device;
		const VkAllocationCallbacks* allocator;

		VkDescriptorPoolDeleter() = default;

		VkDescriptorPoolDeleter(
			VkDevice device,
			const VkAllocationCallbacks* allocator = nullptr
		) noexcept
			: device{ device },
			allocator{ allocator }
		{
		}

		void operator()(VkDescriptorPool descriptorPool) const noexcept
		{
			vkDestroyDescriptorPool(device, descriptorPool, allocator);
		}
	};

	struct VkDeviceDeleter {
		const VkAllocationCallbacks* allocator;

		VkDeviceDeleter(
			const VkAllocationCallbacks* allocator = nullptr
		) noexcept
			: allocator{ allocator }
		{
		}

		void operator()(VkDevice device) const noexcept
		{
			vkDestroyDevice(device, allocator);
		}
	};

	struct VkDeviceMemoryDeleter {
		VkDevice device;
		const VkAllocationCallbacks* allocator;

		VkDeviceMemoryDeleter() = default;

		VkDeviceMemoryDeleter(
			VkDevice device,
			const VkAllocationCallbacks* allocator = nullptr
		) noexcept
			: device{ device },
			allocator{ allocator }
		{
		}

		void operator()(VkDeviceMemory memory) const noexcept
		{
			vkFreeMemory(device, memory, allocator);
		}
	};

	struct VkFenceDeleter {
		VkDevice device;
		const VkAllocationCallbacks* allocator;

		VkFenceDeleter() = default;

		VkFenceDeleter(
			VkDevice device,
			const VkAllocationCallbacks* allocator = nullptr
		) noexcept
			: device{ device },
			allocator{ allocator }
		{
		}

		void operator()(VkFence fence) const noexcept
		{
			vkDestroyFence(device, fence, allocator);
		}
	};

	struct VkFramebufferDeleter {
		VkDevice device;
		const VkAllocationCallbacks* allocator;

		VkFramebufferDeleter() = default;

		VkFramebufferDeleter(
			VkDevice device,
			const VkAllocationCallbacks* allocator = nullptr
		) noexcept
			: device{ device },
			allocator{ allocator }
		{
		}

		void operator()(VkFramebuffer framebuffer) const noexcept
		{
			vkDestroyFramebuffer(device, framebuffer, allocator);
		}
	};

	struct VkImageDeleter {
		VkDevice device;
		const VkAllocationCallbacks* allocator;

		VkImageDeleter() = default;

		VkImageDeleter(
			VkDevice device,
			const VkAllocationCallbacks* allocator = nullptr
		) noexcept
			: device{ device },
			allocator{ allocator }
		{
		}

		void operator()(VkImage image) const noexcept
		{
			vkDestroyImage(device, image, allocator);
		}
	};

	struct VkImageViewDeleter {
		VkDevice device;
		const VkAllocationCallbacks* allocator;

		VkImageViewDeleter() = default;

		VkImageViewDeleter(
			VkDevice device,
			const VkAllocationCallbacks* allocator = nullptr
		) noexcept
			: device{ device },
			allocator{ allocator }
		{
		}

		void operator()(VkImageView imageView) const noexcept
		{
			vkDestroyImageView(device, imageView, allocator);
		}
	};

	struct VkInstanceDeleter final {
		const VkAllocationCallbacks* allocator;

		VkInstanceDeleter(
			const VkAllocationCallbacks* allocator = nullptr
		) noexcept
			: allocator{ allocator }
		{
		}

		void operator()(VkInstance instance) const noexcept
		{
			vkDestroyInstance(instance, allocator);
		}
	};

	struct VkPipelineDeleter {
		VkDevice device;
		const VkAllocationCallbacks* allocator;

		VkPipelineDeleter() = default;

		VkPipelineDeleter(
			VkDevice device,
			const VkAllocationCallbacks* allocator = nullptr
		) noexcept
			: device{ device },
			allocator{ allocator }
		{
		}

		void operator()(VkPipeline pipeline) const noexcept
		{
			vkDestroyPipeline(device, pipeline, allocator);
		}
	};

	struct VkPipelineCacheDeleter {
		VkDevice device;
		const VkAllocationCallbacks* allocator;

		VkPipelineCacheDeleter() = default;

		VkPipelineCacheDeleter(
			VkDevice device,
			const VkAllocationCallbacks* allocator = nullptr
		) noexcept
			: device{ device },
			allocator{ allocator }
		{
		}

		void operator()(VkPipelineCache pipelineCache) const noexcept
		{
			vkDestroyPipelineCache(device, pipelineCache, allocator);
		}
	};

	struct VkPipelineLayoutDeleter {
		VkDevice device;
		const VkAllocationCallbacks* allocator;

		VkPipelineLayoutDeleter() = default;

		VkPipelineLayoutDeleter(
			VkDevice device,
			const VkAllocationCallbacks* allocator = nullptr
		) noexcept
			: device{ device },
			allocator{ allocator }
		{
		}

		void operator()(VkPipelineLayout pipelineLayout) const noexcept
		{
			vkDestroyPipelineLayout(device, pipelineLayout, allocator);
		}
	};

	struct VkRenderPassDeleter {
		VkDevice device;
		const VkAllocationCallbacks* allocator;

		VkRenderPassDeleter() = default;

		VkRenderPassDeleter(
			VkDevice device,
			const VkAllocationCallbacks* allocator = nullptr
		) noexcept
			: device{ device },
			allocator{ allocator }
		{
		}

		void operator()(VkRenderPass renderPass) const noexcept
		{
			vkDestroyRenderPass(device, renderPass, allocator);
		}
	};

	struct VkSamplerDeleter {
		VkDevice device;
		const VkAllocationCallbacks* allocator;

		VkSamplerDeleter() = default;

		VkSamplerDeleter(
			VkDevice device,
			const VkAllocationCallbacks* allocator = nullptr
		) noexcept
			: device{ device },
			allocator{ allocator }
		{
		}

		void operator()(VkSampler sampler) const noexcept
		{
			vkDestroySampler(device, sampler, allocator);
		}
	};

	struct VkSemaphoreDeleter {
		VkDevice device;
		const VkAllocationCallbacks* allocator;

		VkSemaphoreDeleter() = default;

		VkSemaphoreDeleter(
			VkDevice device,
			const VkAllocationCallbacks* allocator = nullptr
		) noexcept
			: device{ device },
			allocator{ allocator }
		{
		}

		void operator()(VkSemaphore semaphore) const noexcept
		{
			vkDestroySemaphore(device, semaphore, allocator);
		}
	};

	struct VkShaderModuleDeleter {
		VkDevice device;
		const VkAllocationCallbacks* allocator;

		VkShaderModuleDeleter() = default;

		VkShaderModuleDeleter(
			VkDevice device,
			const VkAllocationCallbacks* allocator = nullptr
		) noexcept
			: device{ device },
			allocator{ allocator }
		{
		}

		void operator()(VkShaderModule shaderModule) const noexcept
		{
			vkDestroyShaderModule(device, shaderModule, allocator);
		}
	};

	struct VkSurfaceKHRDeleter {
		VkInstance instance;
		const VkAllocationCallbacks* allocator;

		VkSurfaceKHRDeleter() = default;

		VkSurfaceKHRDeleter(
			VkInstance instance,
			const VkAllocationCallbacks* allocator = nullptr
		) noexcept
			: instance{ instance },
			allocator{ allocator }
		{
		}

		void operator()(VkSurfaceKHR surface) const noexcept
		{
			vkDestroySurfaceKHR(instance, surface, allocator);
		}
	};

	struct VkSwapchainKHRDeleter {
		VkDevice device;
		const VkAllocationCallbacks* allocator;

		VkSwapchainKHRDeleter() = default;

		VkSwapchainKHRDeleter(
			VkDevice device,
			const VkAllocationCallbacks* allocator = nullptr
		) noexcept
			: device{ device },
			allocator{ allocator }
		{
		}

		void operator()(VkSwapchainKHR swapchain) const noexcept
		{
			vkDestroySwapchainKHR(device, swapchain, allocator);
		}
	};

	template<typename T>
	struct DefaultDeleter;

	template<>
	struct DefaultDeleter<VkBuffer> {
		using type = VkBufferDeleter;
	};

	template<>
	struct DefaultDeleter<VkCommandPool> {
		using type = VkCommandPoolDeleter;
	};

	template<>
	struct DefaultDeleter<VkDescriptorSetLayout> {
		using type = VkDescriptorSetLayoutDeleter;
	};

	template<>
	struct DefaultDeleter<VkDescriptorSet> {
		using type = VkDescriptorSetDeleter;
	};

	template<>
	struct DefaultDeleter<VkDescriptorPool> {
		using type = VkDescriptorPoolDeleter;
	};

	template<>
	struct DefaultDeleter<VkDevice> {
		using type = VkDeviceDeleter;
	};

	template<>
	struct DefaultDeleter<VkDeviceMemory> {
		using type = VkDeviceMemoryDeleter;
	};

	template<>
	struct DefaultDeleter<VkFence> {
		using type = VkFenceDeleter;
	};

	template<>
	struct DefaultDeleter<VkFramebuffer> {
		using type = VkFramebufferDeleter;
	};

	template<>
	struct DefaultDeleter<VkImage> {
		using type = VkImageDeleter;
	};

	template<>
	struct DefaultDeleter<VkImageView> {
		using type = VkImageViewDeleter;
	};

	template<>
	struct DefaultDeleter<VkInstance> {
		using type = VkInstanceDeleter;
	};

	template<>
	struct DefaultDeleter<VkPipeline> {
		using type = VkPipelineDeleter;
	};

	template<>
	struct DefaultDeleter<VkPipelineCache> {
		using type = VkPipelineCacheDeleter;
	};

	template<>
	struct DefaultDeleter<VkPipelineLayout> {
		using type = VkPipelineLayoutDeleter;
	};

	template<>
	struct DefaultDeleter<VkRenderPass> {
		using type = VkRenderPassDeleter;
	};

	template<>
	struct DefaultDeleter<VkSampler> {
		using type = VkSamplerDeleter;
	};

	template<>
	struct DefaultDeleter<VkSemaphore> {
		using type = VkSemaphoreDeleter;
	};

	template<>
	struct DefaultDeleter<VkShaderModule> {
		using type = VkShaderModuleDeleter;
	};

	template<>
	struct DefaultDeleter<VkSurfaceKHR> {
		using type = VkSurfaceKHRDeleter;
	};

	template<>
	struct DefaultDeleter<VkSwapchainKHR> {
		using type = VkSwapchainKHRDeleter;
	};
}

#endif // !MARCHING_CUBES_CORE_DETAIL_DEFAULT_DELETERS_HPP

