# imgui
cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

set(IMGUISRC
  "src/imgui/imgui.cpp"
  "src/imgui/imgui_demo.cpp"
  "src/imgui/imgui_draw.cpp"
  "src/imgui/imgui_impl_glfw.cpp"
  "src/imgui/imgui_impl_opengl3.cpp"
  "src/imgui/imgui_tables.cpp"
  "src/imgui/imgui_widgets.cpp"
PARENT_SCOPE)
