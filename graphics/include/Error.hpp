#pragma once

#include <glad/gl.h>

void GLAPIENTRY GLErrorCallback(GLenum src, GLenum type, GLuint id, GLenum severity,
								GLsizei length, const GLchar* message,
								const void* userParam);

void GLFWErrorCallback(int errCode, const char* errDesc);
