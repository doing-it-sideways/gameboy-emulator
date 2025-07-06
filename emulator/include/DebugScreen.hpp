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
#define InitDebugScreen(struct GLFWwindow*, const class Memory*) (void*)0
#define UpdateDebugScreenBegin() (void*)0
#define UpdateDebugScreenEnd() (void*)0
#define ShutdownDebugScreen() (void*)0
#endif // DEBUG

} // namespace debug
} // namespace gb
