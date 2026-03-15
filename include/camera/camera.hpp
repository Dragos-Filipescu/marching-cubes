#pragma once
#ifndef MARCHING_CUBES_CAMERA_HPP
#define MARCHING_CUBES_CAMERA_HPP

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/matrix_float4x4.hpp>

#include <camera/frustum.hpp>
#include <collisions/collisions.hpp>
#include <core/aliases.hpp>
#include <scene/transform.hpp>
#include <utils/glsl_alignment.hpp>
#include <utils/utils.hpp>

namespace marching_cubes::camera {

    using scene::BasicTransform;
    using scene::Translation;
    using scene::Rotation;
    using scene::Scale;

    using Transform = BasicTransform<Translation, Rotation, Scale>;

    using collisions::AABB;
    using utils::alignment::AlignedStructMember140;

    struct alignas(
        utils::alignment::GLSLStructAlignment140V<
            glm::vec3,
            glm::vec3,
            glm::vec3,
            glm::vec3
        >
    )
    CameraUBO {
        AlignedStructMember140<glm::vec3> position;
        AlignedStructMember140<glm::vec3> forward;
        AlignedStructMember140<glm::vec3> right;
        AlignedStructMember140<glm::vec3> up;
    };

    class Camera {
    public:
        Camera(
            f32 sensitivityX = 0.001f,
            f32 sensitivityY = 0.001f,
            f32 fovRadians = glm::radians(90.0f),
            f32 aspectRatio = 16.0f / 9.0f,
            f32 near = 0.1f,
            f32 far = 1000.0f
        ) noexcept;

        Camera(const Camera&) = delete;
        Camera& operator=(const Camera&) = delete;

        Camera(Camera&&) = default;
        Camera& operator=(Camera&&) = default;

        virtual ~Camera() = default;

        Camera& setProjection(
            f32 fovRadians,
            f32 aspectRatio,
            f32 nearPlane,
            f32 farPlane
        ) noexcept;

        Camera& translateForward(f32 distance) noexcept;
        Camera& translateUpward(f32 distance) noexcept;
        Camera& translateRight(f32 distance) noexcept;
        Camera& lookAt(
            const glm::vec3& target,
            const glm::vec3& up = kVec3Up
        ) noexcept;
        Camera& processMouseDelta(glm::vec2 delta) noexcept;

        [[nodiscard]] glm::mat4 getViewMatrix() const noexcept;
        [[nodiscard]] glm::mat4 getProjectionMatrix() const noexcept;
        [[nodiscard]] const Transform& getTransform() const noexcept;
        [[nodiscard]] Transform& getTransform() noexcept;
        [[nodiscard]] glm::vec3 getPosition() const noexcept;
        [[nodiscard]] glm::vec3 getForward() const noexcept;
        [[nodiscard]] glm::vec3 getRight() const noexcept;
        [[nodiscard]] glm::vec3 getUp() const noexcept;
        [[nodiscard]] f32 getYaw() const noexcept;
        [[nodiscard]] f32 getPitch() const noexcept;
        [[nodiscard]] f32 getSensitivityX() const noexcept;
        [[nodiscard]] f32 getSensitivityY() const noexcept;
        [[nodiscard]] f32 getFovRadians() const noexcept;
        [[nodiscard]] f32 getAspectRatio() const noexcept;
        [[nodiscard]] f32 getNear() const noexcept;
        [[nodiscard]] f32 getFar() const noexcept;
        [[nodiscard]] Frustum getFrustum() const noexcept;
        [[nodiscard]] AABB getFrustumAABB() const noexcept;

    protected:
        Transform   m_Transform;
        f32         m_Yaw;
        f32         m_Pitch;
		f32         m_Roll;
        f32         m_SensitivityX;
        f32         m_SensitivityY;
        f32         m_FovRadians;
        f32         m_AspectRatio;
        f32         m_Near;
        f32         m_Far;
    };

    [[nodiscard]] CameraUBO ToUBO(const Camera& camera) noexcept;
}

#endif // !MARCHING_CUBES_CAMERA_HPP

