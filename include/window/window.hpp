#pragma	once
#ifndef MARCHING_CUBES_WINDOW_WINDOW_HPP
#define MARCHING_CUBES_WINDOW_WINDOW_HPP

#include <GLFW/glfw3.h>

#include <compare>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <controllers/input_controller.hpp>

namespace marching_cubes::window {

	class Window final {
	public:

		Window() noexcept = default;

		Window(
			const int width,
			const int height,
			const char* title,
			controllers::InputController* inputController,
			const std::vector<std::pair<int, int>>& hints = {},
			GLFWmonitor* monitor = nullptr,
			GLFWwindow* share = nullptr,
			GLFWframebuffersizefun callback = framebufferSizeCallback
		);

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		Window(Window&& other) noexcept;
		Window& operator=(Window&& other) noexcept;

		~Window() noexcept;

		void pollEvents() const noexcept;

		[[nodiscard]] bool shouldClose() const noexcept;

		[[nodiscard]] const GLFWwindow* getWindow() const noexcept;
		[[nodiscard]] GLFWwindow* getWindow() noexcept;
		[[nodiscard]] bool hasResized() const noexcept;
		[[nodiscard]] const controllers::InputController* getInputController() const noexcept;
		[[nodiscard]] controllers::InputController* getInputController() noexcept;

		void setResized() noexcept;
		void resetResized() noexcept;

		constexpr operator const GLFWwindow*() const noexcept
		{
			return m_Window;
		}

		constexpr operator GLFWwindow*() noexcept
		{
			return m_Window;
		}

		constexpr auto operator<=>(GLFWwindow* other) const noexcept
		{
			return m_Window <=> other;
		}

		template<typename U>
			requires std::is_convertible_v<U, GLFWwindow*>
		constexpr auto operator<=>(U other) const noexcept {
			return m_Window <=> static_cast<GLFWwindow*>(other);
		}

	private:

		static void framebufferSizeCallback(GLFWwindow* window, int width, int height) noexcept;

		[[nodiscard]] GLFWwindow* initWindow(
			const int width,
			const int height,
			const char* title,
			GLFWmonitor* monitor,
			GLFWwindow* share,
			const std::vector<std::pair<int, int>>& hints,
			GLFWframebuffersizefun callback
		);

		static void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos) noexcept;
		static void keyPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods) noexcept;
		static void mouseButtonPressCallback(GLFWwindow* window, int button, int action, int mods) noexcept;
		static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) noexcept;
		static void windowFocusCallback(GLFWwindow* window, int focused) noexcept;
		static void windowIconifyCallback(GLFWwindow* window, int iconified) noexcept;

		int									m_Width{};
		int									m_Height{};
		std::string							m_Title{};
		std::vector<std::pair<int, int>>	m_Hints{};
		GLFWmonitor*						m_Monitor{};
		GLFWwindow*							m_Share{};
		GLFWwindow*							m_Window{};
		bool								m_Resized{};
		bool								m_IgnoreNextFocusEvent{ true };
		controllers::InputController*		m_InputController{};
	};
}
#endif // !MARCHING_CUBES_WINDOW_WINDOW_HPP
