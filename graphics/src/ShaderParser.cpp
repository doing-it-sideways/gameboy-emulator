#include "ShaderParser.hpp"

#include <fstream>
#include <sstream>
#include <print>

std::string ShaderFileAsString(const std::filesystem::path& shaderPath) {
	std::ifstream file{ shaderPath };

	if (!file.is_open()) {
		std::println(stderr, "shader file at {} invalid!", shaderPath.string());
		return "";
	}

	std::stringstream buf;
	buf << file.rdbuf();
	return buf.str();
}

GLuint ShaderCompile(GLenum type, const std::string& shader) {
	GLuint shaderId = glCreateShader(type);

	auto src = shader.c_str();
	glShaderSource(shaderId, 1, &src, nullptr);
	glCompileShader(shaderId);

	int ok;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &ok);

	if (!ok) {
		char errInfo[512]{};
		glGetShaderInfoLog(shaderId, 512, nullptr, errInfo);

		std::string_view typeName;
		switch (type) {
		case GL_VERTEX_SHADER:
			typeName = "Vertex";
			break;
		case GL_FRAGMENT_SHADER:
			typeName = "Fragment";
			break;
		default:
			typeName = "Unknown";
			break;
		}

		std::println(stderr, "{} shader failed compilation: {}",
					 type == GL_VERTEX_SHADER ? "Vertex" : "Fragment",
					 errInfo);
	}

	return shaderId;
}

void ShaderLink(GLuint shaderProgram, GLuint vertexShader, GLuint fragShader) {
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragShader);
	glLinkProgram(shaderProgram);

	int ok;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &ok);
	
	if (!ok) {
		char errInfo[512]{};
		glGetProgramInfoLog(shaderProgram, 512, NULL, errInfo);
		std::println(stderr, "Shader program failed linkage: {}", errInfo);
	}
}
