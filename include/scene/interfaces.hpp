#pragma once
#ifndef MARCHING_CUBES_SCENE_INTERFACES_HPP
#define MARCHING_CUBES_SCENE_INTERFACES_HPP

#include <vulkan/vulkan_core.h>

#include <cstddef>
#include <cstdint>
#include <span>

#include <core/aliases.hpp>

namespace marching_cubes::scene {

    struct IMesh {

        virtual ~IMesh() = default;

        virtual void bind(VkCommandBuffer cmd) const noexcept = 0;
        virtual void draw(
            VkCommandBuffer cmd,
            u32 instanceCount = 1,
            u32 firstInstance = 0
        ) const noexcept = 0;

        [[nodiscard]] virtual VkVertexInputBindingDescription
            getBindingDescription(u32 binding = 0) const noexcept = 0;
        [[nodiscard]] virtual std::span<const VkVertexInputAttributeDescription>
            getAttributeDescriptions(u32 binding = 0) const noexcept = 0;
        [[nodiscard]] virtual std::size_t indexCount() const noexcept = 0;
    };

    struct IMaterial {

        virtual ~IMaterial() = default;

        [[nodiscard]] virtual VkPipeline                  getPipeline()           const noexcept = 0;
        [[nodiscard]] virtual VkPipelineLayout            getPipelineLayout()     const noexcept = 0;
        [[nodiscard]] virtual std::span<VkDescriptorSet>  getDescriptorSets()     const noexcept = 0;
    };

    struct Renderable {
        IMesh*      mesh;
        IMaterial*  material;

        void render(VkCommandBuffer cmd) const {
            // 1) bind the pipeline + descriptors
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, material->getPipeline());
            vkCmdBindDescriptorSets(
                cmd,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                material->getPipelineLayout(),
                0,
                static_cast<u32>(material->getDescriptorSets().size()),
                material->getDescriptorSets().data(),
                0,
                nullptr
            );

            // 2) bind the mesh geometry
            mesh->bind(cmd);

            // 3) draw
            mesh->draw(cmd);
        }
    };
}

#endif // !MARCHING_CUBES_SCENE_INTERFACES_HPP

