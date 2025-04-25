#pragma once

#include <GLBuffers.hpp>

namespace gb {

class Screen {
public:
	static constexpr int winWidth = 1280;
	static constexpr int winHeight = 720;
	
	static constexpr int scrWidth = 160;
	static constexpr int scrHeight = 144;
	
	static constexpr float scaleWidth = float(winWidth) / scrWidth;
	static constexpr float scaleHeight = float(winHeight) / scrHeight;

public:
	Screen();
	~Screen();
	
	constexpr Screen(Screen&&) = default;
	constexpr Screen& operator=(Screen&&) = default;

	bool Update();

private:
	GLFWwindow* _window = nullptr;
	cyber::FrameBuffer _fbo{};
};

} // namespace gb
