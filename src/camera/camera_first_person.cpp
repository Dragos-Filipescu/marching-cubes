#include <camera/camera_first_person.hpp>

#include <glm/ext/vector_float3.hpp>

#include <core/aliases.hpp>
#include <utils/utils.hpp>

namespace marching_cubes::camera
{
    CameraFirstPerson::CameraFirstPerson(
        const glm::vec3& pos
    )
    {
        m_Transform.setPosition(pos);
    }

    CameraFirstPerson& CameraFirstPerson::rotateOX(f32 angle)
    {
        m_Transform.rotate(getRight(), angle);
        return *this;
    }

    CameraFirstPerson& CameraFirstPerson::rotateOY(f32 angle)
    {
        m_Transform.rotate(c_Vec3Up, angle);
        return *this;
    }

    CameraFirstPerson& CameraFirstPerson::rotateOZ(f32 angle)
    {
        m_Transform.rotate(getForward(), angle);
        return *this;
    }
}
