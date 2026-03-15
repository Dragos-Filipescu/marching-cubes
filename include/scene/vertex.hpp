#pragma once
#ifndef MARCHING_CUBES_SCENE_VERTEX_HPP
#define MARCHING_CUBES_SCENE_VERTEX_HPP

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan_core.h>

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>

#include <core/aliases.hpp>
#include <utils/glsl_alignment.hpp>
#include <utils/tmp.hpp>
#include <utils/utils.hpp>

namespace marching_cubes::scene {

	using marching_cubes::utils::tmp::indexOf;
	using marching_cubes::utils::tmp::isOneOf;

	template<
		u32 Location,
		typename T,
		VkFormat Format
	>
		requires (
			std::is_trivially_destructible_v<T>
			&& std::is_trivially_copyable_v<T>
		)
	struct VertexAttribute final {
		static constexpr u32 kLocation = Location;
		using Type = T;
		static constexpr VkFormat kFormat = Format;
	};

	namespace detail {
		template<typename T>
		struct IsVertexAttribute : std::false_type {};

		template<
			auto L,
			typename T,
			VkFormat F
		>
		struct IsVertexAttribute<VertexAttribute<L, T, F>> : std::true_type {};
	}

	template<typename T>
	concept VertexAttr = detail::IsVertexAttribute<T>::value;

	enum class VertexPacking : u8 {
		Standard,
		Tight,
	};

	template<VertexPacking Packing, VertexAttr... Attrs>
		requires (sizeof...(Attrs) >= 1)
	struct alignas(Packing == VertexPacking::Standard ? 16 : 4) BasicVertex final {
	private:

		template<std::size_t I>
		static consteval void bump(
			std::size_t& cursor,
			std::array<std::size_t, sizeof...(Attrs)>& offs
		) noexcept
		{
			using A = std::tuple_element_t<I, std::tuple<Attrs...>>;
			using T = typename A::Type;
			/**
			 * The Vulkan API doesn't actually care about the alignment as long as the offsets match,
			 * but we'll use utils::GLSLLayout::Std430, as it matches the expected alignment behaviour.
			 */
			constexpr std::size_t align = utils::alignment::GLSLStructMemberAlignment430V<T>;

			cursor = alignUp<align>(cursor);
			offs[I] = cursor;
			cursor += sizeof(T);
		}

		template<std::size_t... I>
		static consteval auto layoutInfoImpl(std::index_sequence<I...>) {
			constexpr std::size_t N = sizeof...(Attrs);
			std::array<std::size_t, N> offs{};
			std::size_t cursor = 0;

			// fold over all indices, calling bump<I>(cursor, offs)
			(bump<I>(cursor, offs), ...);

			return std::pair{ offs, cursor };
		}

		static consteval auto layoutInfo() {
			return layoutInfoImpl(std::index_sequence_for<Attrs...>{});
		}

	public:

		static constexpr auto kInfo = layoutInfo();

		static constexpr auto kPacking = Packing;
		using Attributes = std::tuple<Attrs...>;

		static constexpr auto kSize = (
			Packing == VertexPacking::Standard
			? alignUp<16>(kInfo.second)
			: alignUp<4>(kInfo.second)
		);
		static constexpr auto kOffsets = kInfo.first;

		BasicVertex()
			: data{}
		{
			[&]<std::size_t... I>(std::index_sequence<I...>) {
				(
					([&]() {
						using T = typename std::tuple_element_t<I, Attributes>::Type;
						T* raw = reinterpret_cast<T*>(data.data() + kOffsets[I]);
						// start the T object here
						std::construct_at(raw);
					}()),
					...
				);
			}(std::index_sequence_for<Attrs...>{});
		}

		template<typename... Args>
		constexpr BasicVertex(Args&&... values) noexcept
			requires (sizeof...(Args) == sizeof...(Attrs))
			&& (std::constructible_from<typename Attrs::Type, Args&&> && ...)
			: BasicVertex{} // zero-initialize `data`
		{
			((get<Attrs>() = std::forward<Args>(values)), ...);
		}

		std::array<std::byte, kSize> data{};

		static constexpr VkVertexInputBindingDescription GetBindingDescription(
			u32 binding = 0
		)
		{
			return VkVertexInputBindingDescription{
				.binding	= binding,
				.stride		= static_cast<u32>(kSize),
				.inputRate	= VK_VERTEX_INPUT_RATE_VERTEX
			};
		}

		static constexpr auto GetAttributeDescriptions(
			u32 binding = 0
		)
		{
			auto result = getCommonAttrs();
			for (VkVertexInputAttributeDescription& attr : result) {
				attr.binding = binding;
			}
			return result;
		}

		static consteval auto getCommonAttrs() noexcept
		{
			return []<std::size_t... I>(std::index_sequence<I...>) {
				return std::array<
					VkVertexInputAttributeDescription,
					sizeof...(Attrs)
				>{
					VkVertexInputAttributeDescription{
						.location = Attrs::kLocation,
						.binding = 0,
						.format = Attrs::kFormat,
						.offset = static_cast<u32>(kOffsets[I])
					}...
				};
			}(std::index_sequence_for<Attrs...>{});
		}

		template<typename Attr>
			requires (isOneOf<Attr, Attrs...>)
		constexpr auto& get() noexcept
		{
			constexpr std::size_t I = indexOf<Attr, Attrs...>();
			using T = typename Attr::Type;
			auto ptr = reinterpret_cast<T*>(data.data() + kOffsets[I]);
			return *std::launder(ptr);
		}

		template<typename Attr>
			requires (isOneOf<Attr, Attrs...>)
		constexpr const auto& get() const noexcept
		{
			constexpr std::size_t I = indexOf<Attr, Attrs...>();
			using T = typename Attr::Type;
			auto ptr = reinterpret_cast<const T*>(data.data() + kOffsets[I]);
			return *std::launder(ptr);
		}

		constexpr bool operator==(const BasicVertex<Packing, Attrs...>& other) const noexcept
		{
			return ((get<Attrs>() == other.template get<Attrs>()) && ...);
		}
	};

	namespace detail {
		template<typename>
		struct IsVertexType : std::false_type {};

		template<VertexPacking Packing, VertexAttr... Attrs>
		struct IsVertexType<BasicVertex<Packing, Attrs...>> : std::true_type {};
	}

	template<typename T>
	concept Vertex = detail::IsVertexType<T>::value;

	template<Vertex V>
	struct VertexHasher {
		constexpr std::size_t operator()(const V& v) const noexcept
		{
			std::size_t seed = 0;
			// Fold expression over the vertex attributes
			([&]<std::size_t... I>(std::index_sequence<I...>) {
				((seed = hash_combine(seed, v.template get<std::tuple_element_t<I, typename V::Attributes>>())), ...);
			})(std::index_sequence_for<typename V::Attributes>{});
			return seed;
		}
	};

	using Position	= VertexAttribute<0, glm::vec3, VK_FORMAT_R32G32B32_SFLOAT>;
	using Normal	= VertexAttribute<1, glm::vec3, VK_FORMAT_R32G32B32_SFLOAT>;
	using Texcoord	= VertexAttribute<2, glm::vec2, VK_FORMAT_R32G32_SFLOAT>;
	using Color		= VertexAttribute<3, glm::vec3, VK_FORMAT_R32G32B32_SFLOAT>;

	using MyVertex = BasicVertex<
		VertexPacking::Standard,
		Position,
		Normal,
		Texcoord,
		Color
	>;

} // namespace marching_cubes

#endif // !MARCHING_CUBES_SCENE_VERTEX_HPP

