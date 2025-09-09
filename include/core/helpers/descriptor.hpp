#pragma once
#ifndef MARCHING_CUBES_CORE_HELPERS_DESCRIPTOR_HPP
#define MARCHING_CUBES_CORE_HELPERS_DESCRIPTOR_HPP

#include <vulkan/vulkan_core.h>
#include <variant>
#include <vector>

#include <core/aliases.hpp>

namespace marching_cubes::core::helpers {

	namespace detail {
		using DescriptorInfo = std::variant<
			VkDescriptorBufferInfo,
			VkDescriptorImageInfo,
			VkBufferView
		>;
	}

	struct DescriptorWriteInfo final {
		VkDescriptorType type;
		u32 binding;
		detail::DescriptorInfo info;
	};

	std::vector<VkWriteDescriptorSet> buildWriteDescriptorSets(
		VkDescriptorSet descriptorSet,
		const std::vector<DescriptorWriteInfo>& infos
	);

	VkDescriptorSet populateDescriptorSet(
		VkDevice device,
		VkDescriptorSet descriptorSet,
		const std::vector<DescriptorWriteInfo>& infos
	);

	[[nodiscard]] VkDescriptorSetLayout createDescriptorSetLayout(
		VkDevice device,
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings,
		VkDescriptorSetLayoutCreateFlags flags = 0,
		const void* pNext = nullptr,
		const VkAllocationCallbacks* pAllocator = nullptr
	);

	[[nodiscard]] VkDescriptorPool createDescriptorPool(
		VkDevice device,
		const std::vector<VkDescriptorPoolSize>& poolSizes,
		u32 maxSets,
		VkDescriptorPoolCreateFlags flags = 0,
		const void* pNext = nullptr,
		const VkAllocationCallbacks* pAllocator = nullptr
	);

	[[nodiscard]] std::vector<VkDescriptorSet> createDescriptorSets(
		VkDevice device,
		VkDescriptorPool descriptorPool,
		const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
		const void* pNext = nullptr
	);

}

#endif // !MARCHING_CUBES_CORE_HELPERS_DESCRIPTOR_HPP

