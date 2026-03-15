#pragma once
#ifndef MARCHING_CUBES_SCENE_TRANSFORM_HPP
#define MARCHING_CUBES_SCENE_TRANSFORM_HPP

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <tuple>

#include <core/aliases.hpp>
#include <core/concepts.hpp>
#include <utils/tmp.hpp>
#include <utils/utils.hpp>

namespace marching_cubes::scene {

	using marching_cubes::utils::tmp::indexOf;
	using marching_cubes::utils::tmp::isOneOf;

	struct Translation {
		using Type = glm::vec3;
		static constexpr std::size_t kSize = sizeof(Type);
	};

	struct Rotation {
		using Type = glm::quat;
		static constexpr std::size_t kSize = sizeof(Type);
	};

	struct Scale {
		using Type = glm::vec3;
		static constexpr std::size_t kSize = sizeof(Type);
	};

	template<typename T>
	concept TransformComponent = isOneOf<T, Translation, Rotation, Scale>;

	template<TransformComponent... Components>
		requires(sizeof...(Components) >= 1 && sizeof...(Components) <= 3)
	class alignas(std::max({ alignof(typename Components::Type)... })) BasicTransform final {

		template<std::size_t I>
		static consteval void bump(
			std::size_t& cursor,
			std::array<std::size_t, sizeof...(Components)>& offs
		) noexcept
		{
			using T = typename std::tuple_element_t<I, std::tuple<Components...>>;
			offs[I] = cursor;
			cursor += T::kSize;
		}

		template<std::size_t... I>
		static consteval auto layoutInfoImpl(std::index_sequence<I...>) {
			constexpr std::size_t N = sizeof...(Components);
			std::array<std::size_t, N> offs{};
			std::size_t cursor = 0;

			// fold over all indices, calling bump<I>(cursor, offs)
			(bump<I>(cursor, offs), ...);

			return std::pair{ offs, cursor };
		}

		static consteval auto layoutInfo() {
			return layoutInfoImpl(std::index_sequence_for<Components...>{});
		}

		static constexpr auto kLayoutInfo = layoutInfo();

		static constexpr auto kOffsets = kLayoutInfo.first;
		static constexpr auto kSize = kLayoutInfo.second;

		template<TransformComponent C>
		static constexpr bool has = isOneOf<C, Components...>;
		
		template<TransformComponent C>
			requires (has<C>)
		[[nodiscard]] constexpr auto& getDataFor() noexcept
		{
			constexpr std::size_t I = indexOf<C, Components...>();
			using T = typename C::Type;
			auto ptr = reinterpret_cast<T*>(mData.data() + kOffsets[I]);
			return *std::launder(ptr);
		}

		template<TransformComponent C>
			requires (has<C>)
		[[nodiscard]] constexpr const auto& getDataFor() const noexcept
		{
			constexpr std::size_t I = indexOf<C, Components...>();
			using T = typename C::Type;
			auto ptr = reinterpret_cast<const T*>(mData.data() + kOffsets[I]);
			return *std::launder(ptr);
		}

	public:

		BasicTransform()
			: mData{}
		{
			[&]<std::size_t... I>(std::index_sequence<I...>) {
				(
					([&]() {
						using T = typename std::tuple_element_t<I, std::tuple<Components...>>::Type;
						T* raw = reinterpret_cast<T*>(mData.data() + kOffsets[I]);
						// start the T object here
						std::construct_at(raw);
					}()),
					...
				);
			}(std::index_sequence_for<Components...>{});
		}

		template<typename... Args>
		constexpr BasicTransform(Args&&... values) noexcept
			requires (
				sizeof...(Args) == sizeof...(Components)
				&& (std::constructible_from<typename Components::Type, Args&&> && ...)
			)
			: BasicTransform{} // zero-initialize `data`
		{
			((getDataFor<Components>() = std::forward<Args>(values)), ...);
		}

		template<TransformComponent... OtherComponents>
			requires (
				sizeof...(OtherComponents) == sizeof...(Components)
				&& (isOneOf<OtherComponents, Components...> && ...)
				&& (isOneOf<Components, OtherComponents...> && ...)
			)
		[[nodiscard]] constexpr bool operator==(const BasicTransform<OtherComponents...>& other) const noexcept
		{
			return ((getDataFor<Components>() == other.template getDataFor<OtherComponents>()) && ...);
		}

		[[nodiscard]] auto getForward() const noexcept
			requires (has<Rotation>)
		{
			return getDataFor<Rotation>() * kVec3Forward;
		}

		[[nodiscard]] auto getRight() const noexcept
			requires (has<Rotation>)
		{
			return getDataFor<Rotation>() * kVec3Right;
		}

		[[nodiscard]] glm::vec3 getUp() const noexcept
			requires (has<Rotation>)
		{
			return getDataFor<Rotation>() * kVec3Up;
		}

		[[nodiscard]] glm::vec3 getPosition() const noexcept
			requires (has<Translation>)
		{
			return getDataFor<Translation>();
		}

		[[nodiscard]] glm::quat getRotation() const noexcept
			requires (has<Rotation>)
		{
			return getDataFor<Rotation>();
		}

		[[nodiscard]] glm::vec3 getScale() const noexcept
			requires (has<Scale>)
		{
			return getDataFor<Scale>();
		}

		auto& setPosition(const glm::vec3& position) noexcept
			requires(has<Translation>)
		{
			getDataFor<Translation>() = position;
			return *this;
		}

		auto& setRotation(const glm::quat& rotation) noexcept
			requires(has<Rotation>)
		{
			getDataFor<Rotation>() = rotation;
			return *this;
		}

		auto& setScale(const glm::vec3& scale) noexcept
			requires(has<Scale>)
		{
			getDataFor<Scale>() = scale;
			return *this;
		}

		auto& setScale(f32 scale) noexcept
			requires(has<Scale>)
		{
			getDataFor<Scale>() = glm::vec3{ scale };
			return *this;
		}

		auto& translate(const glm::vec3& offset) noexcept
			requires(has<Translation>)
		{
			getDataFor<Translation>() += offset;
			return *this;
		}

		auto& rotate(const glm::vec3& axis, f32 angleRadians) noexcept
			requires (has<Rotation>)
		{
			if (glm::length2(axis) < 1e-6f || std::abs(angleRadians) < 1e-6f) {
				return *this;
			}

			auto rot = glm::angleAxis(angleRadians, glm::normalize(axis));
			getDataFor<Rotation>() = glm::normalize(rot * getDataFor<Rotation>());
			return *this;
		}

		auto& scale(const glm::vec3& scale) noexcept
			requires (has<Scale>)
		{
			getDataFor<Scale>() *= scale;
			return *this;
		}

		auto& scale(f32 scale) noexcept
			requires (has<Scale>)
		{
			getDataFor<Scale>() *= glm::vec3{ scale };
			return *this;
		}

		auto& lookAt(const glm::vec3& target, const glm::vec3& up = kVec3Up) noexcept
			requires (has<Translation> && has<Rotation>)
		{
			if (glm::length2(target - getDataFor<Translation>()) < 1e-6f) {
				return *this;
			}

			auto direction = glm::normalize(target - getDataFor<Translation>());
			getDataFor<Rotation>() = glm::quatLookAt(direction, up);
			return *this;
		}

		std::array<std::byte, kSize> mData{};
	};

	template<TransformComponent... Components>
	[[nodiscard]] constexpr auto getModelMatrix(const BasicTransform<Components...>& t) noexcept
	{
		glm::mat4 result{ 1.0f };
		if constexpr (isOneOf<Translation, Components...>)
			result = glm::translate(result, t.getPosition());
		if constexpr (isOneOf<Rotation, Components...>)
			result = result * glm::toMat4(t.getRotation());
		if constexpr (isOneOf<Scale, Components...>)
			result = glm::scale(result, t.getScale());
		return result;
	}
}

#endif // !MARCHING_CUBES_SCENE_TRANSFORM_HPP

