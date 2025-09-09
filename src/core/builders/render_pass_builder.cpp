#include <core/builders/render_pass_builder.hpp>

namespace marching_cubes::core::builders {

    SubpassBuilder::SubpassBuilder(
        RenderPassBuilder& parent,
        VkPipelineBindPoint bindPoint,
        VkSubpassDescriptionFlags flags
    )
        : m_Parent{ parent },
        m_Description{
            .flags = flags,
            .pipelineBindPoint = bindPoint,
        },
        m_InputRefs{},
        m_ColorRefs{},
        m_ResolveRefs{},
        m_DepthRef{},
        m_HasDepth{ false },
        m_Preserve{}
    {
    }

    SubpassBuilder& SubpassBuilder::addInputAttachmentRef(
        VkAttachmentReference attachment
    ) {
        m_InputRefs.push_back(attachment);
        return *this;
    }

    SubpassBuilder& SubpassBuilder::addColorAttachmentRef(
        VkAttachmentReference attachment
    ) {
        m_ColorRefs.push_back(attachment);
        return *this;
    }

    SubpassBuilder& SubpassBuilder::addResolveAttachmentRef(
        VkAttachmentReference attachment
    ) {
        m_ResolveRefs.push_back(attachment);
        return *this;
    }

    SubpassBuilder& SubpassBuilder::setDepthStencilAttachmentRef(
        VkAttachmentReference attachment
    ) noexcept {
        m_DepthRef = attachment;
        m_HasDepth = true;
        return *this;
    }

    SubpassBuilder& SubpassBuilder::preserveAttachment(
        std::uint32_t attachment
    ) {
        m_Preserve.push_back(attachment);
        return *this;
    }

    RenderPassBuilder& SubpassBuilder::endSubpass() {
        m_Description.inputAttachmentCount = static_cast<std::uint32_t>(m_InputRefs.size());
        m_Description.pInputAttachments = m_InputRefs.data();
        m_Description.colorAttachmentCount = static_cast<std::uint32_t>(m_ColorRefs.size());
        m_Description.pColorAttachments = m_ColorRefs.data();
        m_Description.pResolveAttachments = m_ResolveRefs.empty()
            ? nullptr
            : m_ResolveRefs.data();
        m_Description.pDepthStencilAttachment = m_HasDepth
            ? &m_DepthRef
            : nullptr;
        m_Description.preserveAttachmentCount = static_cast<std::uint32_t>(m_Preserve.size());
        m_Description.pPreserveAttachments = m_Preserve.data();

        m_Parent.m_Subpasses.push_back(m_Description);
        m_Parent.m_SubpassData.push_back(
            SubpassData{
                .inputRefs = std::move(m_InputRefs),
                .colorRefs = std::move(m_ColorRefs),
                .resolveRefs = std::move(m_ResolveRefs),
                .depthRef = m_DepthRef,
                .preserves = std::move(m_Preserve),
            }
            );
        return m_Parent;
    }

    RenderPassBuilder::RenderPassBuilder(VkDevice device)
        : m_Device{ device },
        m_Attachments{},
        m_Subpasses{},
        m_SubpassDependencies{}
    {
    }

    RenderPassBuilder& RenderPassBuilder::addAttachment(
        const VkAttachmentDescription& description
    ) {
        m_Attachments.push_back(description);
        return *this;
    }

    SubpassBuilder RenderPassBuilder::beginSubpass(
        VkPipelineBindPoint bindPoint,
        VkSubpassDescriptionFlags flags
    ) {
        return SubpassBuilder{ *this, bindPoint, flags };
    }

    RenderPassBuilder& RenderPassBuilder::addDependency(
        const VkSubpassDependency& dependency
    ) {
        m_SubpassDependencies.push_back(dependency);
        return *this;
    }

    VkRenderPass RenderPassBuilder::build(
        VkRenderPassCreateFlags flags,
        const VkAllocationCallbacks* allocator,
        const void* pNext
    ) {
        if (m_Subpasses.empty()) {
            throw std::runtime_error{ "RenderPass needs at least one subpass." };
        }

        VkRenderPassCreateInfo renderPassInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = pNext,
            .flags = flags,
            .attachmentCount = static_cast<std::uint32_t>(m_Attachments.size()),
            .pAttachments = m_Attachments.data(),
            .subpassCount = static_cast<std::uint32_t>(m_Subpasses.size()),
            .pSubpasses = m_Subpasses.data(),
            .dependencyCount = static_cast<std::uint32_t>(m_SubpassDependencies.size()),
            .pDependencies = m_SubpassDependencies.data()
        };

        VkRenderPass renderPass{};
        if (vkCreateRenderPass(
            m_Device,
            &renderPassInfo,
            allocator,
            &renderPass
        ) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to create render pass!" };
        }
        return renderPass;
    }
}

