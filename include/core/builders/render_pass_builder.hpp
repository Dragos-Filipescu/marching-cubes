#pragma once
#ifndef MARCHING_CUBES_CORE_BUILDERS_RENDER_PASS_BUILDER_HPP
#define MARCHING_CUBES_CORE_BUILDERS_RENDER_PASS_BUILDER_HPP

#include <vulkan/vulkan.h>

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include <core/physical_device.hpp>

namespace marching_cubes::core::builders {

    struct SubpassData {
        std::vector<VkAttachmentReference>  inputRefs{};
        std::vector<VkAttachmentReference>  colorRefs{};
        std::vector<VkAttachmentReference>  resolveRefs{};
        VkAttachmentReference               depthRef{};
        std::vector<std::uint32_t>          preserves{};
    };

    class RenderPassBuilder;

    class SubpassBuilder final {

    public:

        SubpassBuilder& addInputAttachmentRef(
            VkAttachmentReference attachment
        );

        SubpassBuilder& addColorAttachmentRef(
            VkAttachmentReference attachment
        );

        SubpassBuilder& addResolveAttachmentRef(
            VkAttachmentReference attachment
        );

        SubpassBuilder& setDepthStencilAttachmentRef(
            VkAttachmentReference attachment
        ) noexcept;

        SubpassBuilder& preserveAttachment(
            std::uint32_t attachment
        );

        RenderPassBuilder& endSubpass();

    private:

        friend class RenderPassBuilder;

        SubpassBuilder(
            RenderPassBuilder& parent,
            VkPipelineBindPoint bindPoint,
            VkSubpassDescriptionFlags flags
        );

        RenderPassBuilder&                  m_Parent;
        VkSubpassDescription                m_Description;
        std::vector<VkAttachmentReference>  m_InputRefs;
        std::vector<VkAttachmentReference>  m_ColorRefs;
        std::vector<VkAttachmentReference>  m_ResolveRefs;
        VkAttachmentReference               m_DepthRef;
        bool                                m_HasDepth;
        std::vector<std::uint32_t>          m_Preserve;
    };

	class RenderPassBuilder final {
    public:

        RenderPassBuilder(VkDevice device);

        RenderPassBuilder& addAttachment(
            const VkAttachmentDescription& description
        );

        SubpassBuilder beginSubpass(
            VkPipelineBindPoint bindPoint,
            VkSubpassDescriptionFlags flags = 0
        );

        RenderPassBuilder& addDependency(
            const VkSubpassDependency& dependency
        );

        [[nodiscard]] VkRenderPass build(
            VkRenderPassCreateFlags flags = 0,
            const VkAllocationCallbacks* allocator = nullptr,
            const void* pNext = nullptr
        );

    private:

        friend class SubpassBuilder;

        VkDevice                                m_Device;
        std::vector<VkAttachmentDescription>    m_Attachments;
        std::vector<VkSubpassDescription>       m_Subpasses;
        std::vector<VkSubpassDependency>        m_SubpassDependencies;
        std::vector<SubpassData>                m_SubpassData;
	};
}

#endif // !MARCHING_CUBES_CORE_BUILDERS_RENDER_PASS_BUILDER_HPP

