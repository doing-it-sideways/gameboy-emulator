#pragma once

#include <GLBuffers.hpp>

#include <memory>

#include "Core.hpp"

struct GLFWwindow;

namespace gb {

class Screen {
public:	
	static constexpr float scaleWidth = 5;
	static constexpr float scaleHeight = 5;

	static constexpr int winWidth = scrWidth * scaleWidth;
	static constexpr int winHeight = scrHeight * scaleHeight;
public:
	Screen();
	~Screen();
	
	Screen(Screen&&) = default;
	Screen& operator=(Screen&&) = default;

	bool Update();

	inline bool IsClosed() const { return _isClosed; }
	inline GLFWwindow* GetGLFWWindow() const { return _window; }

private:
	void OnFocus(bool focused);
	void OnChangeRes(int widthPixels, int heightPixels);
	inline void OnClose() { _isClosed = true; }

private:
	GLFWwindow* _window = nullptr;
	std::unique_ptr<cyber::FrameBuffer> _fbo{};

	bool _isClosed = false;
};

} // namespace gb
