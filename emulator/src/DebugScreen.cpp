#ifdef DEBUG

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

#include "DebugScreen.hpp"
#include "Memory.hpp"

/*
	A lot of the code here is taken from the Dear ImGui glfw_opengl3 docking branch
	example:
	https://github.com/ocornut/imgui/blob/docking/examples/example_glfw_opengl3/main.cpp

	Also for the VRAM viewer, massive thanks to Low Level Devel on YouTube for the tutorials,
	I couldn't wrap my head around drawing the pixels.
	https://www.youtube.com/watch?v=--Y0J0LRbd0
	or: https://github.com/rockytriton/LLD_gbemu/blob/main/part11/lib/ui.c
*/

namespace gb {
namespace debug {

static const Memory* debugMem = nullptr;
static constexpr float DebugScale = 3;

// gb colors
static constexpr ImColor colors[4] = {
	ImColor{ 1.f, 1.f, 1.f, 1.f },
	ImColor{ .6666f, .6666f, .6666f, 1.f },
	ImColor{ .3333f, .3333f, .3333f, 1.f },
	ImColor{ 0.f, 0.f, 0.f, 1.f }
};

static void VRAMViewer();
static void ScreenViewer();

void InitDebugScreen(GLFWwindow* emuWindow, const Memory* mem) {
	if (debugMem)
		return; // avoid being called twice

	debugMem = mem;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(emuWindow, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.f;
	style.Colors[ImGuiCol_WindowBg].w = 1.f;
}

void UpdateDebugScreenBegin() {
	if (!debugMem)
		return;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::ShowDemoWindow();
	ImGui::ShowMetricsWindow();

	VRAMViewer();

	ImGui::Render();
}

static void VRAMViewer() {
	auto& mem = *debugMem;

	if (!ImGui::Begin("VRAM Viewer"))
		return;

	ImDrawList* dl = ImGui::GetWindowDrawList();
	glm::vec2 start = ImGui::GetCursorScreenPos();

	// thank you mr low level devel for the drawing code :D
	// imgui: asset browser, custom drawing -> primitives

	int tileNum = 0, xDraw = 0, yDraw = 0;
	for (int y = 0; y < 24; ++y) {
		for (int x = 0; x < 16; ++x) {

			const glm::vec2 spritePos = start + glm::vec2{ xDraw + (x * DebugScale), yDraw + (y * DebugScale) };

			for (int tileY = 0; tileY < 16; tileY += 2) {
				const u16 addr = 0x8000 + (tileNum * 16) + tileY;

				byte b1 = mem.DebugReadVRAM(addr);
				byte b2 = mem.DebugReadVRAM(addr + 1);

				for (int bit = 7; bit >= 0; --bit) {
					byte colorIndex = (!!((b1 & (1 << bit))) << 1) | (!!(b2 & (1 << bit)));

					const glm::vec2 tilePos = spritePos + glm::vec2{ 7 - bit, tileY / 2 } *DebugScale;

					dl->AddRectFilled(tilePos, tilePos + DebugScale, colors[colorIndex]);
				}

			}

			xDraw += 8 * DebugScale;
			++tileNum;
		}

		yDraw += 8 * DebugScale;
		xDraw = 0;
	}

	ImGui::End();
}

static void ScreenViewer() {

}

void UpdateDebugScreenEnd() {
	if (!debugMem)
		return;

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	GLFWwindow* backup = glfwGetCurrentContext();
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
	glfwMakeContextCurrent(backup);
}

void ShutdownDebugScreen() {
	if (!debugMem)
		return;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

} // namespace debug
} // namespace gb

#endif // DEBUG