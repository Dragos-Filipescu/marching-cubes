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
        // pitch = rotation around camera's local right axis
        m_Pitch += angle;

        // clamp pitch to avoid upside-down orbits
        static constexpr f32 pitchLimit = glm::radians(89.0f);
        m_Pitch = glm::clamp(m_Pitch, -pitchLimit, pitchLimit);

        updatePosition();
        return *this;
    }

    CameraThirdPerson& CameraThirdPerson::rotateOY(f32 angle)
    {
        // yaw = rotation around world up
        m_Yaw += angle;
        updatePosition();
        return *this;
    }

    CameraThirdPerson& CameraThirdPerson::rotateOZ(f32 angle)
    {
        // Rarely used but consistent
        m_Roll += angle; // if you want roll support
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
        // build orbit orientation from yaw + pitch
        glm::quat qYaw = glm::angleAxis(m_Yaw, kVec3Up);
        glm::quat qPitch = glm::angleAxis(m_Pitch, kVec3Right);

        glm::quat orbitRotation = qYaw * qPitch;

        // offset camera from pivot
        glm::vec3 offset = orbitRotation * glm::vec3(0, 0, m_DistanceToPivot);

        // place camera in world
        m_Transform.setPosition(m_PivotPoint + offset);

        // aim camera at pivot point
        m_Transform.lookAt(m_PivotPoint);
    }
}
