#pragma once
#ifndef MARCHING_CUBES_CONTROLLERS_INPUT_CONTROLLER_HPP
#define MARCHING_CUBES_CONTROLLERS_INPUT_CONTROLLER_HPP

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <array>
#include <vector>

#include <core/aliases.hpp>

namespace marching_cubes::window {
    class Window;
}

namespace marching_cubes::controllers {

    class InputListener;

    class InputController final {

        friend class window::Window;

    public:

        enum class InputFocusState : u8 {
            Active = 0,
            Inactive,
        };

        enum class CaptureMode : u8 {
            None = 0,
            MouseLook,
        };

        enum class KeyState : u8 {
            Up = 0,       // Not pressed
            Pressed,      // Pressed this frame
            Held,         // Held down (after first press)
            Released      // Released this frame
        };

        InputController() noexcept;

        InputController(const InputController&) = delete;
        InputController& operator=(const InputController&) = delete;

        InputController(InputController&&) = default;
        InputController& operator=(InputController&&) = default;

        ~InputController() = default;

        [[nodiscard]] bool isKeyState(int key, KeyState state) const noexcept;
        [[nodiscard]] bool isKeyPressed(int key) const noexcept;
        [[nodiscard]] bool isKeyHeld(int key) const noexcept;
        [[nodiscard]] bool isKeyReleased(int key) const noexcept;
        [[nodiscard]] bool isKeyDown(int key) const noexcept;

        [[nodiscard]] bool isMouseButtonState(int button, KeyState state) const noexcept;
        [[nodiscard]] bool isMouseButtonPressed(int button) const noexcept;
        [[nodiscard]] bool isMouseButtonHeld(int button) const noexcept;
        [[nodiscard]] bool isMouseButtonReleased(int button) const noexcept;
        [[nodiscard]] bool isMouseButtonDown(int button) const noexcept;

        [[nodiscard]] InputFocusState getInputFocus() const noexcept;
        [[nodiscard]] CaptureMode getCaptureMode() const noexcept;

        InputController& setInputFocus(InputFocusState focus) noexcept;
        InputController& setCaptureMode(CaptureMode mode) noexcept;

        InputController& update(f32 deltaTime) noexcept;

        glm::vec2 consumeMouseDelta() noexcept;
        glm::vec2 consumeScrollDelta() noexcept;

        InputController& registerListener(InputListener* listener);
        InputController& unregisterListener(InputListener* listener);

    private:

        void updateKeys() noexcept;
        void updateMouseButtons() noexcept;

        void recomputeInputFocus() noexcept;

        void onKeyPress(int key, int scancode, int action, int mods) noexcept;
        void onMouseButtonPress(int button, int action, int mods) noexcept;
        void onMouseMove(double xpos, double ypos) noexcept;
        void onMouseScroll(double xoffset, double yoffset) noexcept;
        void onWindowFocus(int focused) noexcept;
        void onWindowIconify(int iconified) noexcept;

        glm::vec2   m_MousePosition;
        glm::vec2   m_LastMousePosition;
        glm::vec2   m_MouseDelta;
        glm::vec2   m_ScrollDelta;
        bool        m_FirstMouse;
        bool        m_HasFocus;
        bool        m_IsIconified;

        std::array<KeyState, GLFW_KEY_LAST + 1>             m_KeyStates;
        std::array<KeyState, GLFW_MOUSE_BUTTON_LAST + 1>    m_MouseButtonStates;
        InputFocusState                                     m_InputFocus;
        CaptureMode                                         m_CaptureMode;

        std::vector<InputListener*> m_Listeners;
    };

    class InputListener {

        friend class InputController;

    protected:
        virtual void onKeyPress(
            [[maybe_unused]] int key,
            [[maybe_unused]] InputController::KeyState state,
            [[maybe_unused]] int mods
        ) {}
        virtual void onMouseButtonPress(
            [[maybe_unused]] int button,
            [[maybe_unused]] InputController::KeyState state,
            [[maybe_unused]] int mods
        ) {}
        virtual void onMouseMove(
            [[maybe_unused]] glm::vec2 pos,
            [[maybe_unused]] glm::vec2 delta
        ) {}
        virtual void onMouseScroll(
            [[maybe_unused]] glm::vec2 delta
        ) {}
        virtual void update(
            [[maybe_unused]] f32 deltaTime
        ) {}
        virtual void onWindowFocus(
            [[maybe_unused]] int focused
        ) noexcept {}
        virtual void onWindowIconify(
            [[maybe_unused]] int iconified
        ) noexcept {}

        virtual ~InputListener() = default;
    };
}

#endif // MARCHING_CUBES_CONTROLLERS_INPUT_CONTROLLER_HPP
