
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>

namespace cyber {

template <arithmetic T>
static constexpr GLenum TypeToGLEnum() {
	static_assert(sizeof(T) <= sizeof(GLdouble), "Extended arithmetic types not supported");

	if constexpr (std::is_same_v<T, GLbyte>)
		return GL_BYTE;
	if constexpr (std::is_same_v<T, GLubyte>)
		return GL_UNSIGNED_BYTE;
	else if constexpr (std::is_same_v<T, GLshort>)
		return GL_SHORT;
	else if constexpr (std::is_same_v<T, GLushort>)
		return GL_UNSIGNED_SHORT;
	else if constexpr (std::is_same_v<T, GLint>)
		return GL_INT;
	else if constexpr (std::is_same_v<T, GLuint>)
		return GL_UNSIGNED_INT;
	else if constexpr (std::is_same_v<T, GLfloat>)
		return GL_FLOAT;
	else if constexpr (std::is_same_v<T, GLdouble>)
		return GL_DOUBLE;
	else
		std::unreachable(); // dont.
}

template <std140 T>
VertexBuffer::VertexBuffer(const T* buffer, GLuint bufSize, GLenum drawType)
	: _elements(bufSize)
{
	glGenBuffers(1, &vbo);
	Bind();
	glBufferData(GL_ARRAY_BUFFER, bufSize * sizeof(T), buffer, drawType);
	Unbind();
}

template <std140 T>
VertexBuffer::VertexBuffer(const VertexBuffer& other)
	: _elements(other._elements)
{
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_COPY_READ_BUFFER, other.vbo);
	glBindBuffer(GL_COPY_WRITE_BUFFER, vbo);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, _elements * sizeof(T));
	glBindBuffer(GL_COPY_READ_BUFFER, 0);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
}

template <std140 T>
VertexBuffer& VertexBuffer::operator=(const VertexBuffer& other) {
	if (vbo == other.vbo)
		return *this;

	glBindBuffer(GL_COPY_READ_BUFFER, other.vbo);
	glBindBuffer(GL_COPY_WRITE_BUFFER, vbo);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, _elements * sizeof(T));
	glBindBuffer(GL_COPY_READ_BUFFER, 0);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

	return *this;
}

//template <arithmetic T>
//VertexBuffer::VertexBuffer(GLuint elements, GLenum drawType)
//	: _elements(elements)
//{
//	glGenBuffers(1, &vbo);
//	Bind();
//	glBufferData(GL_ARRAY_BUFFER, elements * sizeof(T), nullptr, drawType);
//}

//template <arithmetic T, typename Alloc>
//VertexBuffer::VertexBuffer(const std::vector<std::vector<T, Alloc>, Alloc>& buffers, GLenum drawType)
//	: VertexBuffer(buffers.size() * buffers[0].size(), drawType)
//{
//	GLintptr offset = 0;
//	for (auto& buf : buffers) {
//		/*glBufferSubData(GL_ARRAY_BUFFER, offset, buf.size() * sizeof(T), buf.data());
//		offset += buf.size() * sizeof(T);*/
//	}
//}

//template <arithmetic T>
//void VertexBuffer::SetData(const T* buffer, GLuint bufSize, GLint bufferNum) {
//	Bind();
//
//	GLint offset = _elements / bufSize * bufferNum;
//	glBufferSubData(GL_ARRAY_BUFFER, offset, bufSize, buffer);
//}

template <std::integral T>
IndexBuffer::IndexBuffer(const T* buffer, GLuint bufSize, GLenum drawType)
	: _count(bufSize)
	, _type(TypeToGLEnum<T>())
{
	glGenBuffers(1, &ebo);
	Bind();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufSize * sizeof(T), buffer, drawType);
	Unbind();
}

template <std140 T>
constexpr AttributeLayout& AttributeLayout::AddAttribute(GLuint vecLen, bool normalized)
{
	if constexpr (is_glm_vec<T>::value) {
		using vecT = is_glm_vec<T>;
		_attribs.emplace_back(TypeToGLEnum<typename T::value_type>(),
							  vecT::len, normalized);
	}
	else if constexpr (is_glm_mat<T>::value) {
		// TODO
		throw std::runtime_error{ "Matrix attributes currently unsupported" };
	}
	else
		_attribs.emplace_back(TypeToGLEnum<T>(), vecLen, normalized);
	
	_stride += vecLen * sizeof(T);

	return *this;
}

template <std140 T>
constexpr AttributeLayout& AttributeLayout::AddNamedAttribute(std::string_view name, GLuint vertexOffset,
															  GLuint vecLen, bool normalized)
{
	AddAttribute<T>(vecLen, normalized);

	auto& attrib = _attribs.back();
	attrib.opt_offset = vertexOffset;
	attrib.opt_name = name;

	return *this;
}

template <std140 T, std::size_t ArrLen>
static constexpr GLsizeiptr UniformBuffer::Getstd140Size() {
	if constexpr (std::is_class_v<T>) {
		if constexpr (is_glm_vec<T>::value) {
			using vecT = is_glm_vec<T>;
			return Getstd140Size<T, vecT::len, vecT::qual>();
		}
		else if constexpr (is_glm_mat<T>::value) {
			using matT = is_glm_mat<T>;
			return Getstd140Size<T, matT::cols, matT::rows, matT::qual>();
		}

		return 0;
		// TODO: Add support for generic classes?
	}
	else if constexpr (std::is_array_v<T>)
		return 16 * ArrLen; // 4N * Size
	else
		return 4;
}

template <std140 T, glm::length_t L, glm::qualifier Q>
static constexpr GLsizeiptr UniformBuffer::Getstd140Size() {
	static_assert(L >= 1 && L <= 4, "Vec length must be under 4");

	if constexpr (L <= 2)
		return 8; // 2N
	else
		return 16; // 4N
}

template <std140 T, glm::length_t C, glm::length_t R, glm::qualifier Q>
static constexpr GLsizeiptr UniformBuffer::Getstd140Size() {
	static_assert(C >= 1 && C <= 4, "Column length must be under 4");
	static_assert(R >= 1 && R <= 4, "Row length must be under 4");

	return 16 * C; // 4N * C
}

template <std140... UniformVals>
static constexpr GLsizeiptr UniformBuffer::Calcstd140Size() {
	return (0 + ... + Getstd140Size<UniformVals>());
}

template <std140 UniformVal>
const UniformBuffer& UniformBuffer::Set(UniformVal value, GLintptr offset) const {
	Bind();

	if constexpr (is_glm_vec<UniformVal>::value || is_glm_mat<UniformVal>::value)
		glBufferSubData(GL_UNIFORM_BUFFER, offset,
						Getstd140Size<UniformVal>(), glm::value_ptr(value));
	else
		glBufferSubData(GL_UNIFORM_BUFFER, offset, Getstd140Size<UniformVal>(), &value);

	Unbind();

	return *this;
}

} // namespace cyber
