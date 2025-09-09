#pragma once
#ifndef MARCHING_CUBES_CAMERA_THIRD_PERSON_HPP
#define MARCHING_CUBES_CAMERA_THIRD_PERSON_HPP

#include <glm/ext/vector_float3.hpp>

#include <camera/camera.hpp>
#include <core/aliases.hpp>

namespace marching_cubes::camera {

    class CameraThirdPerson final : public Camera {
    public:
        CameraThirdPerson(
            const glm::vec3& pivot = glm::vec3{ 0.0f },
            f32 distance = 10.0f
        );

        [[nodiscard]] f32 getDistanceToPivot() const noexcept;
        [[nodiscard]] glm::vec3 getPivotPoint() const noexcept;

        CameraThirdPerson& setPivot(const glm::vec3& pivot);
        CameraThirdPerson& rotateOX(f32 angle);
        CameraThirdPerson& rotateOY(f32 angle);
        CameraThirdPerson& rotateOZ(f32 angle);
        CameraThirdPerson& zoom(f32 delta);

    private:
        void updatePosition();

        f32         m_DistanceToPivot;
        glm::vec3   m_PivotPoint;
    };
}

#endif // !MARCHING_CUBES_CAMERA_THIRD_PERSON_HPP

