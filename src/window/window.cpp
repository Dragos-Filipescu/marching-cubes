#include <window/window.hpp>

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <utility>
#include <vector>

#include <controllers/input_controller.hpp>

namespace marching_cubes::window {

	Window::Window(
		const int width,
		const int height,
		const char* title,
		controllers::InputController* inputController,
		const std::vector<std::pair<int, int>>& hints,
		GLFWmonitor* monitor,
		GLFWwindow* share,
		GLFWframebuffersizefun callback
	) : m_Width{ width },
		m_Height{ height },
		m_Title{ title },
		m_Hints{ hints },
		m_Monitor{ monitor },
		m_Share{ share },
		m_Window{
			initWindow(
				width,
				height,
				title,
				monitor,
				share,
				hints,
				callback
			)
		},
		m_Resized{},
		m_InputController{ inputController }
	{
	}

	Window::Window(Window&& other) noexcept
		: m_Width{ other.m_Width },
		m_Height{ other.m_Height },
		m_Title{ std::move(other.m_Title) },
		m_Hints{ std::move(other.m_Hints) },
		m_Monitor{ other.m_Monitor },
		m_Share{ other.m_Share },
		m_Window{
			initWindow(
				m_Width,
				m_Height,
				m_Title.c_str(),
				m_Monitor,
				m_Share,
				m_Hints,
				framebufferSizeCallback
			),
		},
		m_Resized{ other.m_Resized },
		m_InputController{ other.m_InputController }
	{
	}
	Window& Window::operator=(Window&& other) noexcept
	{
		if (this != &other) {
			if (m_Window) {
				glfwDestroyWindow(m_Window);
			}
			m_Width = other.m_Width;
			m_Height = other.m_Height;
			m_Title = std::move(other.m_Title);
			m_Hints = std::move(other.m_Hints);
			m_Monitor = other.m_Monitor;
			m_Share = other.m_Share;
			m_Window = initWindow(
				m_Width,
				m_Height,
				m_Title.c_str(),
				m_Monitor,
				m_Share,
				m_Hints,
				framebufferSizeCallback
			);
			m_Resized = other.m_Resized;
			m_InputController = other.m_InputController;
		}
		return *this;
	}

	Window::~Window() noexcept
	{
		if (m_Window) {
			glfwDestroyWindow(m_Window);
			m_Window = nullptr;
		}
		m_InputController = nullptr;
	}

	void Window::pollEvents() const noexcept
	{
		glfwPollEvents();
	}

	bool Window::shouldClose() const noexcept
	{
		return glfwWindowShouldClose(m_Window);
	}

	const GLFWwindow* Window::getWindow() const noexcept
	{
		return m_Window;
	}

	GLFWwindow* Window::getWindow() noexcept
	{
		return m_Window;
	}

	bool Window::hasResized() const noexcept
	{
		return m_Resized;
	}

	const controllers::InputController* Window::getInputController() const noexcept
	{
		return m_InputController;
	}

	controllers::InputController* Window::getInputController() noexcept
	{
		return m_InputController;
	}

	void Window::setResized() noexcept
	{
		m_Resized = true;
	}

	void Window::resetResized() noexcept
	{
		m_Resized = false;
	}

	void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) noexcept
	{
		if (width == 0 || height == 0) {
			return;
		}
		glfwSetWindowSize(window, width, height);
		auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
		if (self) {
			self->setResized();
		}
	}

	GLFWwindow* Window::initWindow(
		const int width,
		const int height,
		const char* title,
		GLFWmonitor* monitor,
		GLFWwindow* share,
		const std::vector<std::pair<int, int>>& hints,
		GLFWframebuffersizefun callback
	) {
		for (const auto& [hint, value] : hints) {
			glfwWindowHint(hint, value);
		}
		GLFWwindow* window = glfwCreateWindow(width, height, title, monitor, share);
		if (!window) {
			glfwTerminate();
			throw std::runtime_error{ "Failed to create GLFW window!" };
		}

		glfwSetWindowUserPointer(window, this);

		glfwSetFramebufferSizeCallback(window, callback);
		glfwSetKeyCallback(window, keyPressCallback);
		glfwSetCursorPosCallback(window, mouseMoveCallback);
		glfwSetMouseButtonCallback(window, mouseButtonPressCallback);
		glfwSetScrollCallback(window, scrollCallback);
		glfwSetWindowFocusCallback(window, windowFocusCallback);
		glfwSetWindowIconifyCallback(window, windowIconifyCallback);
		return window;
	}

	void Window::mouseMoveCallback(GLFWwindow* window, double xpos, double ypos) noexcept
	{
		auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
		if (!self) return;
		self->m_InputController->onMouseMove(xpos, ypos);
	}

	void Window::keyPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods) noexcept
	{
		auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
		if (!self) return;
		self->m_InputController->onKeyPress(key, scancode, action, mods);
	}

	void Window::mouseButtonPressCallback(GLFWwindow* window, int button, int action, int mods) noexcept
	{
		auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
		if (!self) return;
		self->m_InputController->onMouseButtonPress(button, action, mods);
	}

	void Window::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) noexcept
	{
		auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
		if (!self) return;
		self->m_InputController->onMouseScroll(xoffset, yoffset);
	}

	void Window::windowFocusCallback(GLFWwindow* window, int focused) noexcept
	{
		auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
		if (self->m_IgnoreNextFocusEvent) {
			self->m_IgnoreNextFocusEvent = false;
			return;
		}
		if (!self) return;
		self->m_InputController->onWindowFocus(focused);
	}

	void Window::windowIconifyCallback(GLFWwindow* window, int iconified) noexcept
	{
		auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
		if (!self) return;
		self->m_InputController->onWindowIconify(iconified);
	}
}
