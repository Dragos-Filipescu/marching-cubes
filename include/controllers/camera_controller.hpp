#pragma once
#ifndef MARCHING_CUBES_CONTROLLERS_CAMERA_CONTROLLER_HPP
#define MARCHING_CUBES_CONTROLLERS_CAMERA_CONTROLLER_HPP

#include <camera/camera.hpp>
#include <camera/camera_first_person.hpp>
#include <controllers/input_controller.hpp>
#include <core/aliases.hpp>

namespace marching_cubes::controllers {

    class CameraController : public InputListener
    {
    public:

        CameraController() noexcept = default;

        CameraController(
            InputController* controller,
            camera::Camera* camera
        )
            : m_Camera{ camera },
            m_Controller{ controller }
        {
            m_Controller->registerListener(this);
        }

        CameraController(const CameraController&) = delete;
        CameraController& operator=(const CameraController&) = delete;

        CameraController(CameraController&& other) noexcept
            : m_Camera{ other.m_Camera },
            m_Controller{ other.m_Controller }
        {
            if (m_Controller) {
                // remove the old registration
                m_Controller->unregisterListener(&other);
                // register the new one
                m_Controller->registerListener(this);
            }
            // 3) leave 'other' in a safe, non-registered state
            other.m_Camera = nullptr;
            other.m_Controller = nullptr;
        }
        CameraController& operator=(CameraController&& other) noexcept
        {
            if (this != &other) {
                // tear down our old registration (if any)
                if (m_Controller) {
                    m_Controller->unregisterListener(this);
                }

                // steal the pointers
                m_Camera = other.m_Camera;
                m_Controller = other.m_Controller;

                // unregister the old object, then register the new one
                if (m_Controller) {
                    m_Controller->unregisterListener(&other);
                    m_Controller->registerListener(this);
                }

                // null out the moved-from object so its dtor is a no-op
                other.m_Camera = nullptr;
                other.m_Controller = nullptr;
            }
            return *this;
        }

        ~CameraController() {
            if (m_Controller) {
                m_Controller->unregisterListener(this);
            }
        }

        void update(f32 deltaTime) override
        {

            constexpr f32 moveSpeed = 10.0f;

            f32 multiplier = 1.0f;

            if (m_Controller->isKeyHeld(GLFW_KEY_LEFT_SHIFT)) {
                // Speed up movement
                multiplier = 2.0f;
            }
            if (m_Controller->isMouseButtonHeld(GLFW_MOUSE_BUTTON_RIGHT)) {
                // Movement
                if (m_Controller->isKeyHeld(GLFW_KEY_W)) {
                    m_Camera->translateForward(moveSpeed * multiplier * deltaTime);
                }
                if (m_Controller->isKeyHeld(GLFW_KEY_S)) {
                    m_Camera->translateForward(-moveSpeed * multiplier * deltaTime);
                }
                if (m_Controller->isKeyHeld(GLFW_KEY_A)) {
                    m_Camera->translateRight(-moveSpeed * multiplier * deltaTime);
                }
                if (m_Controller->isKeyHeld(GLFW_KEY_D)) {
                    m_Camera->translateRight(moveSpeed * multiplier * deltaTime);
                }
                if (m_Controller->isKeyHeld(GLFW_KEY_Q)) {
                    m_Camera->translateUpward(-moveSpeed * multiplier * deltaTime);
                }
                if (m_Controller->isKeyHeld(GLFW_KEY_E)) {
                    m_Camera->translateUpward(moveSpeed * multiplier * deltaTime);
                }

                m_Camera->processMouseDelta(m_Controller->consumeMouseDelta());
            }

            if (m_Controller->isKeyHeld(GLFW_KEY_UP)) {
                static_cast<camera::CameraFirstPerson*>(m_Camera)->rotateOX(glm::radians(30.0f) * deltaTime);
            }
            if (m_Controller->isKeyHeld(GLFW_KEY_DOWN)) {
                static_cast<camera::CameraFirstPerson*>(m_Camera)->rotateOX(-glm::radians(30.0f) * deltaTime);
            }
            if (m_Controller->isKeyHeld(GLFW_KEY_LEFT)) {
                static_cast<camera::CameraFirstPerson*>(m_Camera)->rotateOY(glm::radians(30.0f) * deltaTime);
            }
            if (m_Controller->isKeyHeld(GLFW_KEY_RIGHT)) {
                static_cast<camera::CameraFirstPerson*>(m_Camera)->rotateOY(-glm::radians(30.0f) * deltaTime);
            }
        }

    private:
        camera::Camera* m_Camera{};
        InputController* m_Controller{};
    };
}

#endif // !MARCHING_CUBES_CONTROLLERS_CAMERA_CONTROLLER_HPP

