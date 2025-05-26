#include "Screen.hpp"

#include <GLFW/glfw3.h>
#include <stdexcept>

#include <Error.hpp>

namespace gb {

Screen::Screen()
{
	if (!glfwInit())
		throw std::runtime_error{ "Couldn't initialize glfw" };

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_FLOATING, true);

#ifdef DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

	_window = glfwCreateWindow(winWidth, winHeight, "Gameboy Emulator", nullptr, nullptr);
	if (!_window) {
		glfwTerminate();
		throw std::runtime_error{ "Window couldn't be created." };
	}

	glfwMakeContextCurrent(_window);

	auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(_window, mode->width / 2 - winWidth / 2, mode->height / 2 - winHeight / 2);

	gladLoadGL(glfwGetProcAddress);
	if (GLAD_GL_VERSION_4_6 == 0) {
		glfwTerminate();
		throw std::runtime_error{ "wrong opengl version" };
	}

#ifdef DEBUG
	//glfwSetErrorCallback(GLFWErrorCallback);

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	//glDebugMessageCallback(GLErrorCallback, nullptr);
#endif

	_fbo = std::make_unique<cyber::FrameBuffer>();
	_fbo->AddTextureAttachment(GL_COLOR_ATTACHMENT0, winWidth, winHeight);

	if (!_fbo->CheckComplete()) {
		glfwTerminate();
		throw std::runtime_error{ "Unable to setup framebuffer." };
	}

	glClearColor(1, 1, 1, 1);
}

Screen::~Screen() {
	glfwTerminate();
}

// TODO
bool Screen::Update() {
	// glTexSubImage2D?
	return false;
}

} // namespace gb
