#include <controllers/input_controller.hpp>

#include <algorithm>
#include <utility>
#include <vector>

namespace marching_cubes::controllers {

    // =============================
    // InputController Constructors
    // =============================

    InputController::InputController() noexcept
        : m_MousePosition{},
        m_LastMousePosition{},
        m_MouseDelta{},
        m_ScrollDelta{},
        m_FirstMouse{ true },
        m_HasFocus{ true },
        m_IsIconified{ false },
        m_KeyStates{},
        m_MouseButtonStates{},
        m_InputFocus{ InputFocusState::Active },
        m_CaptureMode{ CaptureMode::MouseLook }
    {
    }

    // =============================
    // Public interface
    // =============================

    bool InputController::isKeyState(int key, KeyState state) const noexcept
    {
        return m_KeyStates[key] == state;
    }

    bool InputController::isKeyPressed(int key) const noexcept
    {
        return m_KeyStates[key] == KeyState::Pressed;
    }

    bool InputController::isKeyHeld(int key) const noexcept
    {
        return m_KeyStates[key] == KeyState::Held;
    }

    bool InputController::isKeyReleased(int key) const noexcept
    {
        return m_KeyStates[key] == KeyState::Released;
    }

    bool InputController::isKeyDown(int key) const noexcept
    {
        const KeyState state = m_KeyStates[key];
        return (state == KeyState::Pressed || state == KeyState::Held);
    }

    bool InputController::isMouseButtonState(int button, KeyState state) const noexcept
    {
        return m_MouseButtonStates[button] == state;
    }

    bool InputController::isMouseButtonPressed(int button) const noexcept
    {
        return m_MouseButtonStates[button] == KeyState::Pressed;
    }

    bool InputController::isMouseButtonHeld(int button) const noexcept
    {
        return m_MouseButtonStates[button] == KeyState::Held;
    }

    bool InputController::isMouseButtonReleased(int button) const noexcept
    {
        return m_MouseButtonStates[button] == KeyState::Released;
    }

    bool InputController::isMouseButtonDown(int button) const noexcept
    {
        const KeyState state = m_MouseButtonStates[button];
        return (state == KeyState::Pressed || state == KeyState::Held);
    }

    InputController::InputFocusState InputController::getInputFocus() const noexcept
    {
        return m_InputFocus;
    }

    InputController::CaptureMode InputController::getCaptureMode() const noexcept
    {
        return m_CaptureMode;
    }

    InputController& InputController::setInputFocus(InputController::InputFocusState focus) noexcept
    {
        m_InputFocus = focus;
        return *this;
    }

    InputController& InputController::setCaptureMode(InputController::CaptureMode mode) noexcept
    {
        m_CaptureMode = mode;
        return *this;
    }

    InputController& InputController::update(float deltaTime) noexcept
    {
        if (m_InputFocus != InputFocusState::Active) {
            return *this;
        }

        updateKeys();
        updateMouseButtons();
        
        for (auto listener : m_Listeners) {
            listener->update(deltaTime);
        }
        
        return *this;
    }

    glm::vec2 InputController::consumeMouseDelta() noexcept
    {
        return std::exchange(m_MouseDelta, glm::vec2{ 0.0f });
    }

    glm::vec2 InputController::consumeScrollDelta() noexcept
    {
        return std::exchange(m_ScrollDelta, glm::vec2{ 0.0f });
    }

    // =============================
    // Private update functions
    // =============================

    void InputController::updateKeys() noexcept
    {
        if (m_InputFocus != InputFocusState::Active) {
            return;
        }

        for (auto& state : m_KeyStates) {
            if (state == KeyState::Pressed) {
                state = KeyState::Held;
            }
            if (state == KeyState::Released) {
                state = KeyState::Up;
            }
        }
    }

    void InputController::updateMouseButtons() noexcept
    {
        if (m_InputFocus != InputFocusState::Active) {
            return;
        }

        for (auto& state : m_MouseButtonStates) {
            if (state == KeyState::Pressed) {
                state = KeyState::Held;
            }
            if (state == KeyState::Released) {
                state = KeyState::Up;
            }
        }
    }

    void InputController::recomputeInputFocus() noexcept
    {
        auto newState = (m_HasFocus && !m_IsIconified)
            ? InputFocusState::Active
            : InputFocusState::Inactive;

        if (newState != m_InputFocus) {
            if (m_InputFocus == InputFocusState::Active && newState == InputFocusState::Inactive) {
                // only clear when *losing* focus
                m_FirstMouse = true;
                m_MouseDelta = {};
                m_ScrollDelta = {};
                std::fill(m_KeyStates.begin(), m_KeyStates.end(), KeyState::Up);
                std::fill(m_MouseButtonStates.begin(), m_MouseButtonStates.end(), KeyState::Up);
            }
            m_InputFocus = newState;
        }
    }

    // =============================
    // Event callbacks (internal)
    // =============================

    void InputController::onKeyPress(
        int key,
        [[maybe_unused]] int scancode,
        int action,
        int mods
    ) noexcept
    {
        if (m_InputFocus != InputFocusState::Active) {
            return;
        }

        if (key < 0 || key > GLFW_KEY_LAST)
            return;

        if (action == GLFW_PRESS) {
            if (m_KeyStates[key] == KeyState::Up || m_KeyStates[key] == KeyState::Released) {
                m_KeyStates[key] = KeyState::Pressed;
            }
        }
        else if (action == GLFW_RELEASE) {
            m_KeyStates[key] = KeyState::Released;
        }

        for (auto listener : m_Listeners) {
            listener->onKeyPress(key, m_KeyStates[key], mods);
        }
    }

    void InputController::onMouseButtonPress(
        int button,
        int action,
        int mods
    ) noexcept
    {
        if (m_InputFocus != InputFocusState::Active) {
            return;
        }

        if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST)
            return;

        if (action == GLFW_PRESS) {
            if (m_MouseButtonStates[button] == KeyState::Up || m_MouseButtonStates[button] == KeyState::Released) {
                m_MouseButtonStates[button] = KeyState::Pressed;
            }
        }
        else if (action == GLFW_RELEASE) {
            m_MouseButtonStates[button] = KeyState::Released;
        }

        for (auto listener : m_Listeners) {
            listener->onMouseButtonPress(button, m_MouseButtonStates[button], mods);
        }
    }

    void InputController::onMouseMove(double xpos, double ypos) noexcept
    {
        if (m_InputFocus != InputFocusState::Active) {
            return;
        }

        const glm::vec2 currentPos{ static_cast<float>(xpos), static_cast<float>(ypos) };

        if (m_FirstMouse) {
            m_LastMousePosition = currentPos;
            m_FirstMouse = false;
            return;
        }

        glm::vec2 delta{ 0.0f };
        if (m_CaptureMode == CaptureMode::MouseLook) {
            delta = currentPos - m_LastMousePosition;
            m_MouseDelta += delta;
        }

        m_LastMousePosition = currentPos;
        m_MousePosition = currentPos;

        for (auto listener : m_Listeners) {
            listener->onMouseMove(currentPos, delta);
        }
    }

    void InputController::onMouseScroll(double xoffset, double yoffset) noexcept
    {
        if (m_InputFocus != InputFocusState::Active) {
            return;
        }

        m_ScrollDelta += glm::vec2{ static_cast<float>(xoffset), static_cast<float>(yoffset) };

        for (auto listener : m_Listeners) {
            listener->onMouseScroll(m_ScrollDelta);
        }
    }

    void InputController::onWindowFocus(int focused) noexcept
    {
        m_HasFocus = static_cast<bool>(focused);
        recomputeInputFocus();
        for (auto listener : m_Listeners) {
            listener->onWindowFocus(focused);
        }
    }

    void InputController::onWindowIconify(int iconified) noexcept
    {
        m_IsIconified = (iconified != 0);
        recomputeInputFocus();
        for (auto listener : m_Listeners) {
            listener->onWindowIconify(iconified);
        }
    }

    // =============================
    // Listener registration
    // =============================

    InputController& InputController::registerListener(InputListener* listener)
    {
        if (std::find(m_Listeners.begin(), m_Listeners.end(), listener)
            == m_Listeners.end()
        ) {
            m_Listeners.emplace_back(listener);
        }
        return *this;
    }

    InputController& InputController::unregisterListener(InputListener* listener)
    {
        std::erase_if(
            m_Listeners,
            [listener](InputListener* ptr) {
                return ptr == listener;
            }
        );
        return *this;
    }
}
