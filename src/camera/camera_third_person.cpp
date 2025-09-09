#include <camera/camera_third_person.hpp>

#include <glm/ext/vector_float3.hpp>

#include <algorithm>

#include <core/aliases.hpp>
#include <utils/utils.hpp>

namespace marching_cubes::camera
{
    CameraThirdPerson::CameraThirdPerson(
        const glm::vec3& pivot,
        f32 distance
    )
        : m_PivotPoint{ pivot },
        m_DistanceToPivot{ distance }
    {
        updatePosition();
    }

    f32 CameraThirdPerson::getDistanceToPivot() const noexcept
    {
        return m_DistanceToPivot;
    }

    glm::vec3 CameraThirdPerson::getPivotPoint() const noexcept
    {
        return m_PivotPoint;
    }

    CameraThirdPerson& CameraThirdPerson::setPivot(const glm::vec3& pivot)
    {
        m_PivotPoint = pivot;
        updatePosition();
        return *this;
    }

    CameraThirdPerson& CameraThirdPerson::rotateOX(f32 angle)
    {
        m_Transform.rotate(getRight(), angle);
        updatePosition();
        return *this;
    }

    CameraThirdPerson& CameraThirdPerson::rotateOY(f32 angle)
    {
        m_Transform.rotate(c_Vec3Up, angle);
        updatePosition();
        return *this;
    }

    CameraThirdPerson& CameraThirdPerson::rotateOZ(f32 angle)
    {
        m_Transform.rotate(getForward(), angle);
        updatePosition();
        return *this;
    }

    CameraThirdPerson& CameraThirdPerson::zoom(f32 delta)
    {
        m_DistanceToPivot = std::max(1.0f, m_DistanceToPivot + delta);
        updatePosition();
        return *this;
    }

    void CameraThirdPerson::updatePosition()
    {
        m_Transform.setPosition(m_PivotPoint - getForward() * m_DistanceToPivot);
    }
}
