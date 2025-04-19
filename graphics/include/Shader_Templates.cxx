#include "glm/gtc/type_ptr.hpp"
#include "TemplateExtensions.hpp"

template <cyber::arithmetic... UniformVals> requires cyber::all_same<UniformVals...>
const Shader& Shader::Set(std::string_view valName, UniformVals... vals) const {
	constexpr auto valNum = sizeof...(UniformVals);
	using UniformType = cyber::firstType_t<UniformVals...>;

	// TODO: support arrays of values
	static_assert(valNum >= 1 && valNum <= 4,
				  "1 <= arguments <= 4 to set a glsl single or vector type.");

	if constexpr (valNum == 1) {
		GLuint loc = GetUniformLocation(valName);
		// even though 1 arg, still have to expand pack
		if constexpr (std::is_floating_point_v<UniformType>)
			glUniform1f(loc, vals...);
		else if constexpr (std::is_unsigned_v<UniformType>)
			glUniform1ui(loc, vals...);
		else
			glUniform1i(loc, vals...);

		return *this;
	}
	else // assumes vector version will be used more often than this
		return Set(valName, glm::vec<valNum, UniformType>(vals...));
}

template <glm::length_t L, cyber::arithmetic UniformVal, glm::qualifier Q>
const Shader& Shader::Set(std::string_view vecName, const glm::vec<L, UniformVal, Q>& vec) const {
	static_assert(L >= 2 && L <= 4, "2 <= vector length <= 4 to set a glsl vector type.");

	GLuint loc = GetUniformLocation(vecName);

	if constexpr (std::is_floating_point_v<UniformVal>) {
		if constexpr (L == 2)
			glUniform2f(loc, vec.x, vec.y);
		else if constexpr (L == 3)
			glUniform3f(loc, vec.x, vec.y, vec.z);
		else if constexpr (L == 4)
			glUniform4fv(loc, 1, glm::value_ptr(vec));
	}
	else if constexpr (std::is_unsigned_v<UniformVal>) {
		if constexpr (L == 2)
			glUniform2ui(loc, vec.x, vec.y);
		else if constexpr (L == 3)
			glUniform3ui(loc, vec.x, vec.y, vec.z);
		else if constexpr (L == 4)
			glUniform4uiv(loc, 1, glm::value_ptr(vec));
	}
	else {
		if constexpr (L == 2)
			glUniform2i(loc, vec.x, vec.y);
		else if constexpr (L == 3)
			glUniform3i(loc, vec.x, vec.y, vec.z);
		else if constexpr (L == 4)
			glUniform4iv(loc, 1, glm::value_ptr(vec));
	}

	return *this;
}

template <glm::length_t C, glm::length_t R, cyber::arithmetic T, glm::qualifier Q>
const Shader& Shader::Set(std::string_view matName, const glm::mat<C, R, T, Q>& mat, bool transpose) const {
	static_assert(C >= 2 && R >= 2, "2 <= matrix dimensions <= 4 to set a glsl type.");
	static_assert(C <= 4 && R <= 4, "2 <= matrix dimensions <= 4 to set a glsl type.");

	GLuint loc = GetUniformLocation(matName);
	auto valuePtr = glm::value_ptr(mat);

	// dimensions same
	if constexpr (C == R) {
		if constexpr (C == 2)
			glUniformMatrix2fv(loc, 1, transpose, valuePtr);	   // 2x2
		else if constexpr (C == 3)
			glUniformMatrix3fv(loc, 1, transpose, valuePtr);	   // 3x3
		else
			glUniformMatrix4fv(loc, 1, transpose, valuePtr);	   // 4x4
	}
	// dimensions different
	else {
		if constexpr (C == 2) {
			if constexpr (R == 3)
				glUniformMatrix2x3fv(loc, 1, transpose, valuePtr); // 2x3
			else
				glUniformMatrix2x4fv(loc, 1, transpose, valuePtr); // 2x4
		}
		else if constexpr (C == 3) {
			if constexpr (R == 2)
				glUniformMatrix3x2fv(loc, 1, transpose, valuePtr); // 3x2
			else
				glUniformMatrix3x4fv(loc, 1, transpose, valuePtr); // 3x4
		}
		else {
			if constexpr (R == 2)
				glUniformMatrix4x2fv(loc, 1, transpose, valuePtr); // 4x2
			else
				glUniformMatrix4x3fv(loc, 1, transpose, valuePtr); // 4x3
		}
	}

	return *this;
}
