#pragma once

#include <string>
#include <filesystem>

#include <glad/gl.h>

std::string ShaderFileAsString(const std::filesystem::path& shaderPath);
GLuint ShaderCompile(GLenum type, const std::string& shader);
void ShaderLink(GLuint shaderProgram, GLuint vertexShader, GLuint fragShader);