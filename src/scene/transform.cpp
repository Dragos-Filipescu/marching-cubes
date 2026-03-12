#include <scene/transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <core/aliases.hpp>

namespace marching_cubes::scene {

	Transform::Transform() noexcept
		: m_Position{ 0.0f },
		m_Rotation{ 1.0f, 0.0f, 0.0f, 0.0f },
		m_Scale{ 1.0f }
	{
	}

	Transform::Transform(
		const glm::vec3& position,
		const glm::quat& rotation,
		const glm::vec3& scale
	) noexcept
		: m_Position{ position },
		m_Rotation{ rotation },
		m_Scale{ scale }
	{
	}

	[[nodiscard]] glm::vec3 Transform::getForward() const noexcept
	{
		return m_Rotation * kVec3Forward;
	}

	[[nodiscard]] glm::vec3 Transform::getRight() const noexcept
	{
		return m_Rotation * kVec3Right;
	}

	[[nodiscard]] glm::vec3 Transform::getUp() const noexcept
	{
		return m_Rotation * kVec3Up;
	}

	[[nodiscard]] glm::vec3 Transform::getPosition() const noexcept
	{
		return m_Position;
	}

	[[nodiscard]] glm::quat Transform::getRotation() const noexcept
	{
		return m_Rotation;
	}

	[[nodiscard]] glm::vec3 Transform::getScale() const noexcept
	{
		return m_Scale;
	}

	[[nodiscard]] glm::mat4 Transform::getModelMatrix() const
	{
		return glm::translate(glm::mat4{ 1.0f }, m_Position)
			* glm::toMat4(m_Rotation)
			* glm::scale(glm::mat4{ 1.0f }, m_Scale);
	}

	Transform& Transform::setPosition(const glm::vec3& position) noexcept
	{
		m_Position = position;
		return *this;
	}

	Transform& Transform::setRotation(const glm::quat& rotation) noexcept
	{
		m_Rotation = rotation;
		return *this;
	}

	Transform& Transform::setScale(const glm::vec3& scale) noexcept
	{
		m_Scale = scale;
		return *this;
	}

	Transform& Transform::setScale(f32 scale) noexcept
	{
		m_Scale = glm::vec3{ scale };
		return *this;
	}

	Transform& Transform::translate(const glm::vec3& offset) noexcept
	{
		m_Position += offset;
		return *this;
	}

	Transform& Transform::rotate(const glm::vec3& axis, f32 angleRadians) noexcept
	{
		if (glm::length2(axis) < 1e-6f || std::abs(angleRadians) < 1e-6f) {
			return *this;
		}

		auto rot = glm::angleAxis(angleRadians, glm::normalize(axis));
		m_Rotation = glm::normalize(rot * m_Rotation);
		return *this;
	}

	Transform& Transform::scale(const glm::vec3& scale) noexcept
	{
		m_Scale *= scale;
		return *this;
	}

	Transform& Transform::scale(f32 scale) noexcept
	{
		m_Scale *= glm::vec3{ scale };
		return *this;
	}

	Transform& Transform::lookAt(const glm::vec3& target, const glm::vec3& up) noexcept
	{
		if (glm::length2(target - m_Position) < 1e-6f) {
			return *this;
		}

		auto direction = glm::normalize(target - m_Position);
		m_Rotation = glm::quatLookAt(direction, up);
		return *this;
	}
}
