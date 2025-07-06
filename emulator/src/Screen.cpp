#include <stdexcept>
#include <utility>

#include <Error.hpp>
#include <GLFW/glfw3.h> // error includes gl.h

#include "Screen.hpp"
#include "DebugScreen.hpp"

template <typename Func, typename... Params>
static void HandleWindowCallback(GLFWwindow* glWindow, Func&& func, Params&&... params) {
	auto* usrPtr = glfwGetWindowUserPointer(glWindow);

	if (gb::Screen* screen = static_cast<gb::Screen*>(usrPtr))
		std::invoke(std::forward<Func>(func), screen, std::forward<Params>(params)...);
}

namespace gb {

Screen::Screen()
{
	if (!glfwInit())
		throw std::runtime_error{ "Couldn't initialize glfw" };

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

	_window = glfwCreateWindow(winWidth, winHeight, "Gameboy Emulator", nullptr, nullptr);
	if (!_window) {
		glfwTerminate();
		throw std::runtime_error{ "Window couldn't be created." };
	}

	glfwSetWindowSizeLimits(_window, scrWidth, scrHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);

	const auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(_window, mode->width / 2 - winWidth / 2, mode->height / 2 - winHeight / 2);

	glfwMakeContextCurrent(_window);
	glfwSetWindowUserPointer(_window, this);

	glfwSetWindowCloseCallback(_window, [](GLFWwindow* win) {
		HandleWindowCallback(win, &Screen::OnClose);
	});
	glfwSetFramebufferSizeCallback(_window, [](GLFWwindow* win, int width, int height) {
		HandleWindowCallback(win, &Screen::OnChangeRes, width, height);
	});
	glfwSetWindowFocusCallback(_window, [](GLFWwindow* win, int focused) {
		HandleWindowCallback(win, &Screen::OnFocus, static_cast<bool>(focused));
	});
	
	gladLoadGL(glfwGetProcAddress);
	if (GLAD_GL_VERSION_4_6 == 0) {
		glfwTerminate();
		throw std::runtime_error{ "wrong opengl version" };
	}

#ifdef DEBUG
	glfwSetErrorCallback(GLFWErrorCallback);

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(GLErrorCallback, nullptr);
#endif

	_fbo = std::make_unique<cyber::FrameBuffer>();
	_fbo->AddTextureAttachment(GL_COLOR_ATTACHMENT0, winWidth, winHeight);

	if (!_fbo->CheckComplete()) {
		glfwTerminate();
		throw std::runtime_error{ "Unable to setup framebuffer." };
	}

	glClearColor(0, 0, 0, 0);
}

Screen::~Screen() {
	debug::ShutdownDebugScreen();

	glfwDestroyWindow(_window);
	glfwTerminate();
}

// TODO
bool Screen::Update() {
	if (glfwWindowShouldClose(_window))
		return false;

	glfwPollEvents();
	debug::UpdateDebugScreenBegin();

	// TODO
	// glTexSubImage2D?
	glClear(GL_COLOR_BUFFER_BIT);

	debug::UpdateDebugScreenEnd();
	glfwSwapBuffers(_window);

	return true;
}

void Screen::OnChangeRes(int widthPixels, int heightPixels) {
	if (glfwWindowShouldClose(_window))
		return;

	glViewport(0, 0, widthPixels, heightPixels);
}

void Screen::OnFocus(bool focused) {
	// TODO
}

} // namespace gb
