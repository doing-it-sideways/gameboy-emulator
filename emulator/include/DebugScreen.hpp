#pragma once

struct GLFWwindow;

namespace gb {

class Memory;

namespace debug {

#ifdef DEBUG
void InitDebugScreen(GLFWwindow* emuWindow, const Memory* mem);
void UpdateDebugScreenBegin();
void UpdateDebugScreenEnd();
void ShutdownDebugScreen();
#else // DEBUG
inline void InitDebugScreen(GLFWwindow*, const Memory*) {}
inline void UpdateDebugScreenBegin() {}
inline void UpdateDebugScreenEnd() {}
inline void ShutdownDebugScreen() {}
#endif // DEBUG

} // namespace debug
} // namespace gb
