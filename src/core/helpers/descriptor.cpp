#include <core/helpers/descriptor.hpp>

#include <vulkan/vulkan_core.h>

#include <stdexcept>
#include <type_traits>
#include <variant>
#include <vector>

#include <core/aliases.hpp>

namespace marching_cubes::core::helpers {

    std::vector<VkWriteDescriptorSet> buildWriteDescriptorSets(
        VkDescriptorSet descriptorSet,
        const std::vector<DescriptorWriteInfo>& infos
    ) {
        std::vector<VkWriteDescriptorSet> writes;
        writes.reserve(infos.size());

        for (const auto& info : infos) {
            VkWriteDescriptorSet write{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptorSet,
                .dstBinding = info.binding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = info.type,
            };

            std::visit(
                [&](auto&& arg) noexcept {
                    using T = std::remove_cvref_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, VkDescriptorBufferInfo>) {
                        write.pBufferInfo = &arg;
                    }
                    else if constexpr (std::is_same_v<T, VkDescriptorImageInfo>) {
                        write.pImageInfo = &arg;
                    }
                    else if constexpr (std::is_same_v<T, VkBufferView>) {
                        write.pTexelBufferView = &arg;
                    }
                },
                info.info
            );

            writes.emplace_back(write);
        }
        return writes;
    }

    VkDescriptorSet populateDescriptorSet(
        VkDevice device,
        VkDescriptorSet descriptorSet,
        const std::vector<DescriptorWriteInfo>& infos
    ) {
        const auto& writes = buildWriteDescriptorSets(descriptorSet, infos);
        vkUpdateDescriptorSets(
            device,
            static_cast<u32>(writes.size()),
            writes.data(),
            0,
            VK_NULL_HANDLE
        );
        return descriptorSet;
    }

    VkDescriptorSetLayout createDescriptorSetLayout(
        VkDevice device,
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings,
        VkDescriptorSetLayoutCreateFlags flags,
        const void* pNext,
        const VkAllocationCallbacks* pAllocator
    ) {
        VkDescriptorSetLayoutCreateInfo layoutInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = pNext,
            .flags = flags,
            .bindingCount = static_cast<u32>(layoutBindings.size()),
            .pBindings = layoutBindings.data(),
        };

        VkDescriptorSetLayout descriptorSetLayout{};
        if (vkCreateDescriptorSetLayout(
            device,
            &layoutInfo,
            pAllocator,
            &descriptorSetLayout
        ) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to create descriptor set layout!" };
        }
        return descriptorSetLayout;
    }

    VkDescriptorPool createDescriptorPool(
        VkDevice device,
        const std::vector<VkDescriptorPoolSize>& poolSizes,
        u32 maxSets,
        VkDescriptorPoolCreateFlags flags,
        const void* pNext,
        const VkAllocationCallbacks* pAllocator
    ) {
        VkDescriptorPoolCreateInfo poolInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = pNext,
            .flags = flags,
            .maxSets = maxSets,
            .poolSizeCount = static_cast<u32>(poolSizes.size()),
            .pPoolSizes = poolSizes.data()
        };

        VkDescriptorPool descriptorPool{};
        if (vkCreateDescriptorPool(
            device,
            &poolInfo,
            pAllocator,
            &descriptorPool
        ) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to create descriptor pool!" };
        }
        return descriptorPool;
    }

    std::vector<VkDescriptorSet> createDescriptorSets(
        VkDevice device,
        VkDescriptorPool descriptorPool,
        const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
        const void* pNext
    ) {
        std::size_t count = descriptorSetLayouts.size();
        VkDescriptorSetAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = pNext,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = static_cast<u32>(count),
            .pSetLayouts = descriptorSetLayouts.data()
        };

        std::vector<VkDescriptorSet> descriptorSets(count);
        if (vkAllocateDescriptorSets(
            device,
            &allocInfo,
            descriptorSets.data()
        ) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to allocate descriptor sets!" };
        }
        return descriptorSets;
    }
}
