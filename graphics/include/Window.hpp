#pragma once

#include <glm/vec2.hpp>
#include <string_view>

#include "ClassConstructorMacros.hpp"

struct GLFWwindow;
struct GLFWmonitor;

class Window {
public:
	enum class Mode : unsigned char { Windowed, WindowedFS, Fullscreen };

public:
	Window(std::string_view name, Mode mode,
		   GLFWmonitor* fsMonitor = nullptr,
		   int defaultWidth = 1280, int defaultHeight = 720);
	~Window();

	DEFAULT_COPY(Window, delete);
	DEFAULT_MOVE(Window, default);

	inline GLFWwindow* GetGLFWWindow() const noexcept { return _glwindow; };
	inline explicit operator GLFWwindow*() const noexcept { return _glwindow; }

	inline GLFWmonitor* GetCurrentMonitor() const noexcept { return _glmonitor; }
	inline explicit operator GLFWmonitor*() const noexcept { return _glmonitor; }

	inline glm::ivec2 Size() const noexcept { return _viewportSize; }

	inline bool IsClosed() const noexcept { return _isClosed; }

	void Update();

	inline void Close() { glfwSetWindowShouldClose(_glwindow, true); }
	void SwapMode(Mode newMode);
	void ChangeRes(int width, int height);
	void EnableVSync(bool enable);

private:
	void UpdateSCRes(int width, int height);
	void UpdateMonitor(int posX, int posY);
	void OnFocus(bool focused);
	void OnMaximize(bool maximized);
	inline void OnClose() { _isClosed = true; }

private:
	GLFWmonitor* _glmonitor;
	GLFWwindow* _glwindow;

	glm::ivec2 _windowSize, _viewportSize{};
	glm::ivec2 _windowPos{};

	Mode _mode;
	bool _vsyncOn = false;
	bool _isClosed = false;
};
