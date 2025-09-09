#pragma once
#ifndef MARCHING_CUBES_CAMERA_FRUSTUM_HPP
#define MARCHING_CUBES_CAMERA_FRUSTUM_HPP

#include <glm/glm.hpp>

#include <array>

#include <collisions/collisions.hpp>
#include <core/aliases.hpp>

namespace marching_cubes::camera {

	using collisions::AABB;
	using collisions::Sphere;

	struct Frustum final {

        std::array<glm::vec4, 6> planes{};

		[[nodiscard]] constexpr bool intersects(const AABB& aabb, f32 epsilon = 0.0f) const
        {
            const glm::vec3 center = 0.5f * (aabb.min + aabb.max);  // box center
            const glm::vec3 extents = 0.5f * (aabb.max - aabb.min); // half extents

            for (const auto& plane : planes) {
                const glm::vec3 planeNormal = glm::vec3{ plane };
                const f32 planeOffset = plane.w;
                const f32 signedDistance = glm::dot(planeNormal, center) + planeOffset;
                const f32 projectionRadius = glm::dot(glm::abs(planeNormal), extents);
				// normals point "inwards", so we check if the signed
                // distance is < -(radius + epsilon) instead of > (radius + epsilon)
                if (signedDistance < -(projectionRadius + epsilon)) {
                    // completely outside this plane
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]] constexpr bool intersects(const Sphere& sphere, f32 epsilon = 0.0f) const
        {
            for (const auto& plane : planes) {
                const glm::vec3 planeNormal = glm::vec3{ plane };
                const f32 planeOffset = plane.w;
                const f32 signedDistance = glm::dot(planeNormal, sphere.center) + planeOffset;
                // normals point "inwards", so we check if the signed
                // distance is < -(radius + epsilon) instead of > (radius + epsilon)
                if (signedDistance < -(sphere.radius + epsilon)) {
                    // completely outside this plane
                    return false;
                }
            }
            return true;
        }
	};
}

#endif // !MARCHING_CUBES_CAMERA_FRUSTUM_HPP

