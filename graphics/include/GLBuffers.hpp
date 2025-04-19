#pragma once

#include <glad/gl.h>

#include <array>
#include <cstdint>
#include <vector>
#include <string_view>
#include <glm/glm.hpp>

#include "ClassConstructorMacros.hpp"
#include "ConceptExtensions.hpp"

class Shader;

namespace cyber {

struct VertexBuffer {
	GLuint vbo = 0;

	template <std140 T>
	VertexBuffer(const T* buffer, GLuint bufSize, GLenum drawType = GL_STATIC_DRAW);

	template <std140 T, GLuint Size>
	inline VertexBuffer(const std::array<T, Size>& buffer, GLenum drawType = GL_STATIC_DRAW)
		: VertexBuffer(buffer.data(), Size, drawType)
	{}

	template <std140 T, typename Alloc = std::allocator<T>>
	inline VertexBuffer(const std::vector<T, Alloc>& buffer, GLenum drawType = GL_STATIC_DRAW)
		: VertexBuffer(buffer.data(), buffer.size(), drawType)
	{}

	~VertexBuffer();

	template <std140 T>
	VertexBuffer(const VertexBuffer&);
	template <std140 T>
	VertexBuffer& operator=(const VertexBuffer&);

	DEFAULT_MOVE(VertexBuffer, default);

	inline operator GLuint() { return vbo; }

	inline void Bind() const { glBindBuffer(GL_ARRAY_BUFFER, vbo); }
	inline void Unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }

	inline GLuint Elements() const { return _elements; }

private:
	GLuint _elements = 0;
};

struct IndexBuffer {
	GLuint ebo = 0;

	template <std::integral T>
	IndexBuffer(const T* buffer, GLuint bufSize, GLenum drawType = GL_STATIC_DRAW);

	template <std::integral T, GLuint Size>
	inline IndexBuffer(const std::array<T, Size>& buffer, GLenum drawType = GL_STATIC_DRAW)
		: IndexBuffer(buffer.data(), Size, drawType)
	{}

	template <std::integral T, typename Alloc = std::allocator<T>>
	inline IndexBuffer(const std::vector<T, Alloc>& buffer, GLenum drawType = GL_STATIC_DRAW)
		: IndexBuffer(buffer.data(), buffer.size(), drawType)
	{}

	~IndexBuffer();

	DEFAULT_COPY(IndexBuffer, delete);
	DEFAULT_MOVE(IndexBuffer, default);

	inline operator GLuint() { return ebo; }

	inline void Bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); }
	inline void Unbind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

	void Draw(GLenum primitive = GL_TRIANGLES, GLsizei instances = 1) const;

private:
	GLuint _count;
	GLenum _type;
};

struct AttributeLayout {
	friend struct VertexArray;

	AttributeLayout(GLuint instanceSeparation = 0);

	template <std140 T>
	constexpr AttributeLayout& AddAttribute(GLuint vecLen = 1, bool normalized = false);

	template <std140 T>
	constexpr AttributeLayout& AddNamedAttribute(std::string_view name, GLuint vertexOffset,
												 GLuint vecLen = 1, bool normalized = false);

	constexpr inline void Reset() { _attribs.clear(); _stride = 0; }

private:
	struct Attribute {
		GLenum type;
		GLushort count; // better packing, num should always be <= 4
		bool normalized;
		// 1 byte padding

		GLuint opt_offset = 0; // required if name provided
		std::string_view opt_name = ""; // if provided
	};

private:
	std::vector<Attribute> _attribs{};
	GLuint _stride = 0;
	GLuint _instanceSeparation;
};

struct VertexArray {
	GLuint vao = 0;

	VertexArray();
	~VertexArray();

	DEFAULT_COPY(VertexArray, delete);
	DEFAULT_MOVE(VertexArray, default);

	inline operator GLuint() { return vao; }

	void SetAttributes(const VertexBuffer& vbo, const AttributeLayout& layout);

	// requires all attributes in the vertexbufferlayout to have been set through AddNamedAttribute
	void SetNamedAttributes(const Shader& shader, const VertexBuffer& vbo, const AttributeLayout& layout);

	inline void Bind() const { glBindVertexArray(vao); }
	inline void Unbind() const { glBindVertexArray(0); }

	void Draw(GLenum primitive, GLint offset, GLsizei count, GLsizei instances = 1) const;
	void DrawKnown(GLsizei instances = 1, GLenum primitive = GL_TRIANGLES, GLuint offset = 0) const;

private:
	GLuint _vertices = 0;
	GLuint _attributes = 0;
};

struct FrameBuffer {
	GLuint fbo = 0;

	FrameBuffer();
	~FrameBuffer();

	DEFAULT_COPY(FrameBuffer, delete);
	DEFAULT_MOVE(FrameBuffer, default);

	inline operator GLuint() { return fbo; }

	inline void Bind() const { glBindFramebuffer(GL_FRAMEBUFFER, fbo); }
	inline void Unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

	GLuint AddTextureAttachment(GLenum attachment, GLsizei width, GLsizei height,
									  GLint filter = GL_NEAREST, GLint wrap = GL_CLAMP_TO_EDGE,
									  GLint lodLevel = 0, bool includeColorAlpha = true);
	GLuint AddRenderBufferAttachment(GLenum attachment, GLsizei width, GLsizei height, GLenum format = 0);

	bool CheckComplete() const;

private:
	std::vector<GLuint> texAttachments{};
	std::vector<GLuint> renderAttachments{};
};

struct UniformBuffer {
	GLuint ubo;

	template <std140 T, std::size_t ArrLen = 0>
	static constexpr GLsizeiptr Getstd140Size();

	template <std140 T, glm::length_t L, glm::qualifier Q>
	static constexpr GLsizeiptr Getstd140Size();

	template <std140 T, glm::length_t C, glm::length_t R, glm::qualifier Q>
	static constexpr GLsizeiptr Getstd140Size();

	template <std140... UniformVals>
	static constexpr GLsizeiptr Calcstd140Size();

	UniformBuffer(GLuint bindingPoint, GLsizeiptr std140Size);

	~UniformBuffer();

	DEFAULT_COPY(UniformBuffer, delete);
	DEFAULT_MOVE(UniformBuffer, default);

	inline operator GLuint() { return ubo; }

	inline void Bind() const { glBindBuffer(GL_UNIFORM_BUFFER, ubo); }
	inline void Unbind() const { glBindBuffer(GL_UNIFORM_BUFFER, 0); }

	template <std140 UniformVal>
	const UniformBuffer& Set(UniformVal value, GLintptr offset) const;
};

} // namespace cyber

#include "GLBuffers_Templates.cxx"
