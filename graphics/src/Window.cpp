#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <print>
#include <stdexcept>
#include <utility>

#include "Window.hpp"
#include "Error.hpp"

template <typename... Params>
static void HandleWindowCallback(GLFWwindow* glwindow, auto&& func, Params... params) {
	auto* window = glfwGetWindowUserPointer(glwindow);
	
	if (Window* win = static_cast<Window*>(window))
		std::invoke(std::forward<decltype(func)>(func), win, std::forward<Params>(params)...);
}

Window::Window(std::string_view name, Mode mode,
			   GLFWmonitor* fsMonitor,
			   int defaultWidth, int defaultHeight)
	: _glmonitor(fsMonitor)
	, _glwindow(nullptr)
	, _windowSize({ defaultWidth, defaultHeight })
	, _mode(mode)
{
	if (!glfwInit())
		throw std::runtime_error{ "Couldn't initialize glfw." };

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	glfwWindowHint(GLFW_FLOATING, true);

	_glwindow = glfwCreateWindow(defaultWidth, defaultHeight, name.data(), fsMonitor, nullptr);
	if (!_glwindow) {
		glfwTerminate();
		throw std::runtime_error{ "Unable to create a glfw window." };
	}

	glfwSetWindowAspectRatio(_glwindow, 16, 9);
	glfwSetWindowSizeLimits(_glwindow, defaultWidth, defaultHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);

	const GLFWvidmode* vidMode = glfwGetVideoMode(_glmonitor ? _glmonitor : glfwGetPrimaryMonitor());

	if (_mode == Mode::Windowed) {
		_windowPos = { vidMode->width / 2 - _windowSize.x / 2, vidMode->height / 2 - _windowSize.y / 2 };
		glfwSetWindowPos(_glwindow, _windowPos.x, _windowPos.y);
	}
	else {
		_windowSize = { vidMode->width, vidMode->height };
		glfwSetWindowSize(_glwindow, _windowSize.x, _windowSize.y);
	}

	glfwGetFramebufferSize(_glwindow, &_viewportSize.x, &_viewportSize.y);

	glfwMakeContextCurrent(_glwindow);

#pragma region Setup Callbacks
	glfwSetWindowUserPointer(_glwindow, this);

	glfwSetFramebufferSizeCallback(_glwindow, [](GLFWwindow* win, int width, int height) {
		HandleWindowCallback(win, &Window::ChangeRes, width, height);
	});
	glfwSetWindowPosCallback(_glwindow, [](GLFWwindow* win, int xPos, int yPos) {
		HandleWindowCallback(win, &Window::UpdateMonitor, xPos, yPos);
	});
	glfwSetWindowFocusCallback(_glwindow, [](GLFWwindow* win, int focused) {
		HandleWindowCallback(win, &Window::OnFocus, focused);
	});
	glfwSetWindowCloseCallback(_glwindow, [](GLFWwindow* win) {
		HandleWindowCallback(win, &Window::OnClose);
	});
	glfwSetWindowMaximizeCallback(_glwindow, [](GLFWwindow* win, int maximized) {
		HandleWindowCallback(win, &Window::OnMaximize, maximized);
	});
#pragma endregion
}

Window::~Window() {
	glfwDestroyWindow(_glwindow);
}

void Window::Update() {
	glfwSwapBuffers(_glwindow);
	glfwPollEvents();
}

void Window::SwapMode(Mode newMode) {
	if (glfwWindowShouldClose(_glwindow))
		return;

	if (newMode == _mode)
		return;

	const GLFWvidmode* mode = glfwGetVideoMode(_glmonitor);

	switch (newMode) {
	case Mode::Windowed:
		glfwSetWindowAttrib(_glwindow, GLFW_FLOATING, false);
		// TODO: make width, height, and refresh rate variable
		glfwSetWindowMonitor(_glwindow, nullptr,
							 _windowPos.x, _windowPos.y,
							 _windowSize.x, _windowSize.y,
							 mode->refreshRate);
		break;
	case Mode::WindowedFS:
		glfwSetWindowAttrib(_glwindow, GLFW_FLOATING, true);
		// TODO: make refresh rate variable
		glfwSetWindowMonitor(_glwindow, _glmonitor,
							 0, 0,
							 mode->width, mode->height,
							 mode->refreshRate);
		glfwSwapInterval(_vsyncOn ? 1 : 0);
		break;
	case Mode::Fullscreen:
		glfwSetWindowAttrib(_glwindow, GLFW_FLOATING, false);
		// TODO: make refresh rate variable
		glfwSetWindowMonitor(_glwindow, _glmonitor,
							 0, 0,
							 mode->width, mode->height,
							 mode->refreshRate);
		glfwSwapInterval(_vsyncOn ? 1 : 0);
		break;
	default:
		std::unreachable();
	}

	_mode = newMode;
}

void Window::UpdateSCRes(int widthSC, int heightSC) {
	if (glfwWindowShouldClose(_glwindow))
		return;

	_windowSize = { widthSC, heightSC };
}

void Window::ChangeRes(int widthPixels, int heightPixels) {
	if (glfwWindowShouldClose(_glwindow))
		return;

	_viewportSize = { widthPixels, heightPixels };
	glViewport(0, 0, widthPixels, heightPixels);
}

void Window::OnMaximize(bool maximized) {
	// TODO
}

void Window::UpdateMonitor(int posX, int posY) {
	if (glfwWindowShouldClose(_glwindow))
		return;

	_windowPos = { posX, posY };

		// In case monitor it's on has udpated as well.
	_glmonitor = glfwGetWindowMonitor(_glwindow);
}

void Window::OnFocus(bool focused) {
	// TODO
}

void Window::EnableVSync(bool enable) {
	if (_mode == Mode::Windowed) {
		std::println(stderr, "VSync can't be enabled on a windowed monitor.");
		return;
	}

	_vsyncOn = enable;
	glfwSwapInterval(enable);
}
