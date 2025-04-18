#pragma once

#include <filesystem>
#include <string_view>
#include <print>

#include <glad/gl.h>
#include <glm/glm.hpp>

#include "ConceptExtensions.hpp"
#include "ClassConstructorMacros.hpp"

class Shader {
public:
	Shader(const std::filesystem::path& vertPath, const std::filesystem::path& fragPath);
	~Shader();

	DEFAULT_COPY(Shader, delete);
	DEFAULT_MOVE(Shader, default);

	inline void Use() const { glUseProgram(programID); }

	template <cyber::arithmetic... Ts> requires cyber::all_same<Ts...>
	const Shader& Set(std::string_view valName, Ts... vals) const;

	template <glm::length_t L, cyber::arithmetic T, glm::qualifier Q>
	const Shader& Set(std::string_view vecName, const glm::vec<L, T, Q>& vec) const;

	template <glm::length_t C, glm::length_t R, cyber::arithmetic T, glm::qualifier Q>
	const Shader& Set(std::string_view matName, const glm::mat<C, R, T, Q>& mat, bool transpose = false) const;

	inline const Shader& SetSampler(std::string_view samplerName, int val) const { Set<int>(samplerName, val); return *this; }

	GLint GetUniformLocation(std::string_view uniformName) const;

private:
	GLuint programID = -1;
};

#include "Shader_Templates.cxx"
