#include "Shader.hpp"
#include "ShaderParser.hpp"

Shader::Shader(const std::filesystem::path& vertPath, const std::filesystem::path& fragPath) {
	GLuint vertShader = ShaderCompile(GL_VERTEX_SHADER, ShaderFileAsString(vertPath));
	GLuint fragShader = ShaderCompile(GL_FRAGMENT_SHADER, ShaderFileAsString(fragPath));

	programID = glCreateProgram();
	ShaderLink(programID, vertShader, fragShader);

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
}

Shader::~Shader() {
	glDeleteProgram(programID);
}

GLint Shader::GetUniformLocation(std::string_view uniformName) const {
	GLint loc = glGetUniformLocation(programID, uniformName.data());
	
	if (loc == -1)
		std::println(stderr, "location of {} not found", uniformName);

	return loc;
}
