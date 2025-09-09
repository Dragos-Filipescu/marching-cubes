#ifndef MARCHING_CUBES_COLLISIONS_AABB_HPP
#define MARCHING_CUBES_COLLISIONS_AABB_HPP

#include <glm/glm.hpp>

#include <core/aliases.hpp>

namespace marching_cubes::collisions {

	struct AABB final {

		glm::vec3 min{};
		glm::vec3 max{};
	};

	struct Sphere final {
		glm::vec3 center{};
		f32 radius{};
	};

	[[nodiscard]] constexpr bool intersects(
		const AABB& aabb1,
		const AABB& aabb2
	) noexcept
	{
		return (aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) &&
			(aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y) &&
			(aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z);
	}

	[[nodiscard]] constexpr bool intersects(
		const Sphere& sphere,
		const AABB& aabb
	)
	{
		glm::vec3 closestPoint{};
		closestPoint.x = glm::clamp(sphere.center.x, aabb.min.x, aabb.max.x);
		closestPoint.y = glm::clamp(sphere.center.y, aabb.min.y, aabb.max.y);
		closestPoint.z = glm::clamp(sphere.center.z, aabb.min.z, aabb.max.z);
		glm::vec3 diff = closestPoint - sphere.center;
		float distSq = glm::dot(diff, diff);
		return distSq <= (sphere.radius * sphere.radius);
	}

	[[nodiscard]] constexpr bool intersects(
		const AABB& aabb,
		const Sphere& sphere
	)
	{
		return intersects(sphere, aabb);
	}

	[[nodiscard]] constexpr bool intersects(
		const Sphere& sphere1,
		const Sphere& sphere2
	)
	{
		glm::vec3 diff = sphere1.center - sphere2.center;
		float distSq = glm::dot(diff, diff);
		float radiusSum = sphere1.radius + sphere2.radius;
		return distSq <= (radiusSum * radiusSum);
	}

	[[nodiscard]] constexpr bool contains(
		const AABB& aabb,
		const glm::vec3& point
	) noexcept
	{
		return (point.x >= aabb.min.x && point.x <= aabb.max.x) &&
			(point.y >= aabb.min.y && point.y <= aabb.max.y) &&
			(point.z >= aabb.min.z && point.z <= aabb.max.z);
	}

	[[nodiscard]] constexpr bool contains(
		const Sphere& sphere,
		const glm::vec3& point
	)
	{
		glm::vec3 diff = sphere.center - point;
		float distSq = glm::dot(diff, diff);
		return distSq <= (sphere.radius * sphere.radius);
	}
}

#endif // !MARCHING_CUBES_COLLISIONS_AABB_HPP
