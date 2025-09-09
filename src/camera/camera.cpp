#include <camera/camera.hpp>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/quaternion_float.hpp>

#include <array>

#include <camera/frustum.hpp>
#include <collisions/collisions.hpp>
#include <core/aliases.hpp>

#include <utils/utils.hpp>

namespace marching_cubes::camera {

    using collisions::AABB;

    Camera::Camera(
        f32 sensitivityX,
        f32 sensitivityY,
        f32 fovRadians,
        f32 aspectRatio,
        f32 nearPlane,
        f32 farPlane
    ) noexcept
        : m_Transform{},
        m_Yaw{},
        m_Pitch{},
        m_SensitivityX{ sensitivityX },
        m_SensitivityY{ sensitivityY},
        m_FovRadians{ fovRadians },
        m_AspectRatio{ aspectRatio },
        m_Near{ nearPlane },
        m_Far{ farPlane }
    {
    }

    Camera& Camera::setProjection(
        f32 fovRadians,
        f32 aspectRatio,
        f32 nearPlane,
        f32 farPlane
    ) noexcept
    {
        m_FovRadians = fovRadians;
        m_AspectRatio = aspectRatio;
        m_Near = nearPlane;
        m_Far = farPlane;
        return *this;
    }

    Camera& Camera::translateForward(f32 distance) noexcept
    {
        m_Transform.translate(getForward() * distance);
        return *this;
    }

    Camera& Camera::translateUpward(f32 distance) noexcept
    {
        m_Transform.translate(getUp() * distance);
        return *this;
    }

    Camera& Camera::translateRight(f32 distance) noexcept
    {
        m_Transform.translate(getRight() * distance);
        return *this;
    }

    Camera& Camera::lookAt(
        const glm::vec3& target,
        const glm::vec3& up
    ) noexcept
    {
        m_Transform.lookAt(target, up);
        return *this;
    }

    Camera& Camera::processMouseDelta(glm::vec2 delta) noexcept
    {
        delta *= glm::vec2{ m_SensitivityX, m_SensitivityY };

        m_Yaw -= delta.x;
        m_Pitch -= delta.y;

        // Clamp pitch to prevent flipping
        static constexpr auto pitchLimit = glm::radians(89.0f);
        m_Pitch = glm::clamp(m_Pitch, -pitchLimit, pitchLimit);

        // Recompute rotation
        glm::quat qYaw = glm::angleAxis(m_Yaw, c_Vec3Up);
        glm::quat qPitch = glm::angleAxis(m_Pitch, c_Vec3Right);
        glm::quat rotation = qYaw* qPitch;

        m_Transform.setRotation(rotation);
        return *this;
    }

    glm::mat4 Camera::getViewMatrix() const noexcept
    {
        return glm::lookAt(
            getPosition(),
            getPosition() + getForward(),
            getUp()
        );
    }

    glm::mat4 Camera::getProjectionMatrix() const noexcept
    {
        glm::mat4 proj = glm::perspective(m_FovRadians, m_AspectRatio, m_Near, m_Far);
        proj[1][1] *= -1.0f; // Vulkan Y-flip correction
        return proj;
    }

    const scene::Transform& Camera::getTransform() const noexcept
    {
        return m_Transform;
    }

    scene::Transform& Camera::getTransform() noexcept
    {
        return m_Transform;
    }

    glm::vec3 Camera::getPosition() const noexcept
    {
        return m_Transform.getPosition();
    }

    glm::vec3 Camera::getForward() const noexcept
    {
        return m_Transform.getRotation() * c_Vec3Forward;
    }

    glm::vec3 Camera::getRight() const noexcept
    {
        return m_Transform.getRotation() * c_Vec3Right;
    }

    glm::vec3 Camera::getUp() const noexcept
    {
        return m_Transform.getRotation() * c_Vec3Up;
    }

    f32 Camera::getYaw() const noexcept
    {
        return m_Yaw;
    }

    f32 Camera::getPitch() const noexcept
    {
        return m_Pitch;
    }

    f32 Camera::getSensitivityX() const noexcept
    {
        return m_SensitivityX;
    }

    f32 Camera::getSensitivityY() const noexcept
    {
        return m_SensitivityY;
    }

    f32 Camera::getFovRadians() const noexcept
    {
        return m_FovRadians;
    }

    f32 Camera::getAspectRatio() const noexcept
    {
        return m_AspectRatio;
    }

    f32 Camera::getNear() const noexcept
    {
        return m_Near;
    }

    f32 Camera::getFar() const noexcept
    {
        return m_Far;
    }

    Frustum Camera::getFrustum() const noexcept
    {
        const glm::mat4 VP = getProjectionMatrix() * getViewMatrix();

        // Reconstruct the ROW vectors from a column-major matrix
        const glm::vec4 r0{ VP[0][0], VP[1][0], VP[2][0], VP[3][0] };
        const glm::vec4 r1{ VP[0][1], VP[1][1], VP[2][1], VP[3][1] };
        const glm::vec4 r2{ VP[0][2], VP[1][2], VP[2][2], VP[3][2] };
        const glm::vec4 r3{ VP[0][3], VP[1][3], VP[2][3], VP[3][3] };

        auto normPlane = [](glm::vec4 p) {
            f32 len = glm::length(glm::vec{ p });
            return (len > 0.0f) ? (p / len) : p;
        };

        Frustum f{};
        // Inward pointing normals for Vulkan/D3D clip space (z in [0,1])
        f.planes[0] = normPlane(r3 + r0); // Left
        f.planes[1] = normPlane(r3 - r0); // Right
        f.planes[2] = normPlane(r3 + r1); // Bottom
        f.planes[3] = normPlane(r3 - r1); // Top
        f.planes[4] = normPlane(r2); // Near
        f.planes[5] = normPlane(r3 - r2); // Far
        return f;
    }


    AABB Camera::getFrustumAABB() const noexcept
    {
        glm::mat4 invVP = glm::inverse(getProjectionMatrix() * getViewMatrix());

        // Define the 8 NDC corners of the canonical frustum
        static constexpr std::array<glm::vec4, 8> ndc = {
            {
                { -1, -1, 0, 1 },
                { +1, -1, 0, 1 },
                { -1, +1, 0, 1 },
                { +1, +1, 0, 1 },
                { -1, -1, 1, 1 },
                { +1, -1, 1, 1 },
                { -1, +1, 1, 1 },
                { +1, +1, 1, 1 },
            }
        };

        // 4) Unproject them and build the world‑axis AABB
        glm::vec3 minPt{};
        glm::vec3 maxPt{};
        for (std::size_t i = 0; i < ndc.size(); ++i) {
            glm::vec4 worldCoord = invVP * ndc[i];
            glm::vec3 pt = glm::vec3{ worldCoord } / worldCoord.w;    // perspective divide

            if (i == 0) {
                minPt = maxPt = pt;
            }
            else {
                minPt = glm::min(minPt, pt);
                maxPt = glm::max(maxPt, pt);
            }
        }

        return AABB{
            .min = minPt,
            .max = maxPt
        };
    }


    CameraUBO ToUBO(const Camera& camera) noexcept
    {
        return CameraUBO{
            .position = camera.getPosition(),
            .forward = camera.getForward(),
            .right = camera.getRight(),
            .up = camera.getUp()
        };
    }
}
