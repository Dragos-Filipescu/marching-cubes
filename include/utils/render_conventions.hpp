#pragma once
#ifndef MARCHING_CUBES_UTILS_RENDER_CONVENTIONS_HPP
#define MARCHING_CUBES_UTILS_RENDER_CONVENTIONS_HPP

#include <vulkan/vulkan_core.h>

#include <concepts>

namespace marching_cubes::utils::render_conventions {

    struct RenderConventions final {
        static constexpr VkFrontFace kFrontFace = VK_FRONT_FACE_CLOCKWISE;
        static constexpr VkCullModeFlags kCullMode = VK_CULL_MODE_BACK_BIT;

        template<std::unsigned_integral IndexT>
            requires (sizeof(IndexT) == 2) || (sizeof(IndexT) == 4)
        struct IndexType final {
            using Type = IndexT;
            static constexpr VkIndexType kVkIndexType = (sizeof(IndexT) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
		};

        template<std::unsigned_integral IndexT>
            requires (sizeof(IndexT) == 2) || (sizeof(IndexT) == 4)
        using IndexTypeT = IndexType<IndexT>::Type;
    };
}

#endif // !MARCHING_CUBES_UTILS_RENDER_CONVENTIONS_HPP

