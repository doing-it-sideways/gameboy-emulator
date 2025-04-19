#include "GLBuffers.hpp"

#include <cassert>
#include <ranges>

#include "Shader.hpp"

namespace cyber {

VertexBuffer::~VertexBuffer() {
	glDeleteBuffers(1, &vbo);
}

IndexBuffer::~IndexBuffer() {
	glDeleteBuffers(1, &ebo);
}

VertexArray::VertexArray() {
	glGenVertexArrays(1, &vao);
}

VertexArray::~VertexArray() {
	glDeleteVertexArrays(1, &vao);
}

FrameBuffer::FrameBuffer() {
	glGenFramebuffers(1, &fbo);
}

FrameBuffer::~FrameBuffer() {
	for (GLuint tex : texAttachments) {
		glDeleteTextures(1, &tex);
	}

	for (GLuint rendBuf : renderAttachments) {
		glDeleteRenderbuffers(1, &rendBuf);
	}

	glDeleteFramebuffers(1, &fbo);
}

UniformBuffer::UniformBuffer(GLuint bindingPoint, GLsizeiptr std140Size) {
	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	// just allocating data
	glBufferData(GL_UNIFORM_BUFFER, std140Size, nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, ubo, 0, std140Size);
}

UniformBuffer::~UniformBuffer() {
	glDeleteBuffers(1, &ubo);
}

static constexpr GLuint GLTypeSize(GLenum type) {
	switch (type) {
	case GL_BYTE:
	case GL_UNSIGNED_BYTE:
		return sizeof(GLbyte);
	case GL_SHORT:
	case GL_UNSIGNED_SHORT:
		return sizeof(GLshort);
	case GL_INT:
	case GL_UNSIGNED_INT:
		return sizeof(GLint);
	case GL_FLOAT:
		return sizeof(GLfloat);
	case GL_DOUBLE:
		return sizeof(GLdouble);
	default:
		std::unreachable(); // man...
	}
}

AttributeLayout::AttributeLayout(GLuint instanceSeparation)
	: _instanceSeparation(instanceSeparation)
{}

void VertexArray::SetAttributes(const VertexBuffer& vbo, const AttributeLayout& layout) {
	vbo.Bind();
	Bind();

	std::size_t offset = 0, attributeTotal = 0;
	for (auto [i, attrib] : std::views::enumerate(layout._attribs)) {
		glEnableVertexAttribArray(i + _attributes);
		glVertexAttribPointer(i + _attributes, attrib.count, attrib.type, attrib.normalized,
							  layout._stride, (const void*)offset);

		if (layout._instanceSeparation != 0)
			glVertexAttribDivisor(i + _attributes, layout._instanceSeparation);

		attributeTotal += attrib.count;
		offset += attrib.count * GLTypeSize(attrib.type);
	}

	// the vertex amount should not be affected by instance attributes
	if (layout._instanceSeparation == 0)
		_vertices += vbo.Elements() / attributeTotal;

	_attributes += layout._attribs.size();

	Unbind();
	vbo.Unbind();
}

void VertexArray::SetNamedAttributes(const Shader& shader,
									 const VertexBuffer& vbo,
									 const AttributeLayout& layout)
{
	vbo.Bind();
	Bind();

	GLsizei attributeTotal = 0;
	GLint maxAttribOffset = 0;
	for (const auto& attrib : layout._attribs) {
		assert(attrib.opt_name != "");

		GLint loc = shader.GetUniformLocation(attrib.opt_name);
		std::size_t longOffset = attrib.opt_offset;

		glEnableVertexAttribArray(loc);
		glVertexAttribPointer(loc, attrib.count, attrib.type, attrib.normalized,
							  layout._stride, (const void*)longOffset);

		if (layout._instanceSeparation != 0)
			glVertexAttribDivisor(loc, layout._instanceSeparation);

		attributeTotal += attrib.count;
		maxAttribOffset = std::max(maxAttribOffset, loc);
	}

	// the vertex amount should not be affected by instance attributes
	if (layout._instanceSeparation == 0)
		_vertices += vbo.Elements() / attributeTotal;

	_attributes = std::max<GLuint>(_attributes, maxAttribOffset);

	Unbind();
	vbo.Unbind();
}

void VertexArray::Draw(GLenum primitive, GLint offset, GLsizei count, GLsizei instances) const {
	assert(instances >= 1);
	
	Bind();

	if (instances == 1)
		glDrawArrays(primitive, offset, count);
	else
		glDrawArraysInstanced(primitive, offset, count, instances);

	Unbind();
}

void VertexArray::DrawKnown(GLsizei instances, GLenum primitive, GLuint offset) const {
	assert(instances >= 1);
	
	Bind();

	if (instances == 1)
		glDrawArrays(primitive, offset, _vertices - offset);
	else
		glDrawArraysInstanced(primitive, offset, _vertices - offset, instances);

	Unbind();
}

void IndexBuffer::Draw(GLenum primitive, GLsizei instances) const {
	assert(instances >= 1);
	
	Bind();

	if (instances == 1)
		glDrawElements(primitive, _count, _type, nullptr);
	else
		glDrawElementsInstanced(primitive, _count, _type, nullptr, instances);
	
	Unbind();
}

GLuint FrameBuffer::AddTextureAttachment(GLenum attachment, GLsizei width, GLsizei height,
											   GLint filter, GLint wrap,
											   GLint lodLevel, bool includeColorAlpha)
{
	GLuint fbTex;
	glGenTextures(1, &fbTex);
	glBindTexture(GL_TEXTURE_2D, fbTex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter == GL_LINEAR_MIPMAP_LINEAR ? GL_LINEAR : filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	
	GLenum internalFormat, pixelFormat, type;
	switch (attachment) {
	case GL_DEPTH_STENCIL_ATTACHMENT:
		internalFormat = GL_DEPTH24_STENCIL8;
		pixelFormat = GL_DEPTH_STENCIL;
		type = GL_UNSIGNED_INT_24_8;
		break;

	case GL_DEPTH_ATTACHMENT:
		internalFormat = GL_DEPTH_COMPONENT;
		pixelFormat = GL_DEPTH_COMPONENT;
		type = GL_UNSIGNED_BYTE;
		break;
	
	default: // GL_COLOR_ATTACHMENT(N)
		if (includeColorAlpha) {
			internalFormat = GL_RGBA;
			pixelFormat = GL_RGBA;
		}
		else {
			internalFormat = GL_RGB;
			pixelFormat = GL_RGB;
		}

		type = GL_UNSIGNED_BYTE;
		break;
	}

	glTexImage2D(GL_TEXTURE_2D, lodLevel, internalFormat, width, height, 0, pixelFormat, type, nullptr);
	
	Bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, fbTex, lodLevel);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	Unbind();

	texAttachments.push_back(fbTex);

	return fbTex;
}

GLuint FrameBuffer::AddRenderBufferAttachment(GLenum attachment, GLsizei width, GLsizei height, GLenum format) {
	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);

	// try and determine a reasonable format ourselves if one isn't provided
	if (format == 0) {
		GLenum internalFormat;
		switch (attachment) {
		case GL_DEPTH_STENCIL_ATTACHMENT:
			internalFormat = GL_DEPTH24_STENCIL8;
			break;
		case GL_DEPTH_ATTACHMENT:
			internalFormat = GL_DEPTH_COMPONENT;
			break;
		default: // GL_COLOR_ATTACHMENT(N)
			internalFormat = GL_RGBA;
			break;
		}

		glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
	}
	else
		glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);

	Bind();
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rbo);
	
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	Unbind();

	renderAttachments.push_back(rbo);

	return rbo;
}

bool FrameBuffer::CheckComplete() const {
	Bind();
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	Unbind();

	return status == GL_FRAMEBUFFER_COMPLETE;
}

} // namespace cyber