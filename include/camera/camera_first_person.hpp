#pragma once
#ifndef MARCHING_CUBES_CAMERA_FIRST_PERSON_HPP
#define MARCHING_CUBES_CAMERA_FIRST_PERSON_HPP

#include <glm/ext/vector_float3.hpp>

#include <camera/camera.hpp>
#include <core/aliases.hpp>

namespace marching_cubes::camera {

    class CameraFirstPerson final : public Camera {
    public:
        CameraFirstPerson(
            const glm::vec3& pos = glm::vec3{ 0.0f }
        );

        CameraFirstPerson& rotateOX(f32 angle);
        CameraFirstPerson& rotateOY(f32 angle);
        CameraFirstPerson& rotateOZ(f32 angle);
    };

} // namespace t2::camera

#endif // !MARCHING_CUBES_CAMERA_FIRST_PERSON_HPP

