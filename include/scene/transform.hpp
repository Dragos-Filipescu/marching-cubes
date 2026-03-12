#pragma once
#ifndef MARCHING_CUBES_SCENE_TRANSFORM_HPP
#define MARCHING_CUBES_SCENE_TRANSFORM_HPP

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <core/aliases.hpp>
#include <utils/utils.hpp>

namespace marching_cubes::scene {

	class Transform final {
	public:
		Transform() noexcept;

		Transform(
			const glm::vec3& position,
			const glm::quat& rotation,
			const glm::vec3& scale
		) noexcept;

		[[nodiscard]] glm::vec3 getForward() const noexcept;
		[[nodiscard]] glm::vec3 getRight() const noexcept;
		[[nodiscard]] glm::vec3 getUp() const noexcept;

		[[nodiscard]] glm::vec3 getPosition() const noexcept;
		[[nodiscard]] glm::quat getRotation() const noexcept;
		[[nodiscard]] glm::vec3 getScale() const noexcept;

		[[nodiscard]] glm::mat4 getModelMatrix() const;

		Transform& setPosition(const glm::vec3& position) noexcept;
		Transform& setRotation(const glm::quat& rotation) noexcept;
		Transform& setScale(const glm::vec3& scale) noexcept;
		Transform& setScale(f32 scale) noexcept;

		Transform& translate(const glm::vec3& offset) noexcept;
		Transform& rotate(const glm::vec3& axis, f32 angleRadians) noexcept;
		Transform& scale(const glm::vec3& scale) noexcept;
		Transform& scale(f32 scale) noexcept;
		Transform& lookAt(const glm::vec3& target, const glm::vec3& up = kVec3Up) noexcept;

	private:
		glm::vec3 m_Position;
		glm::quat m_Rotation;
		glm::vec3 m_Scale;
	};
}

#endif // !MARCHING_CUBES_SCENE_TRANSFORM_HPP

