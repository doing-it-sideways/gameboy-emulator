#include "Error.hpp"

#include <print>

void GLAPIENTRY GLErrorCallback(GLenum src, GLenum type, GLuint id, GLenum severity,
                                GLsizei length, const GLchar* msg,
                                [[maybe_unused]] const void* userParam)
{
    std::basic_string_view<GLchar> _msg(msg, length);
    std::string_view _src, _type, _severity;

    switch (src) {
    case GL_DEBUG_SOURCE_API:
        _src = "API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        _src = "Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        _src = "Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        _src = "Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        _src = "Application";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        _src = "!!Other Source!!";
        break;
    default:
        _src = "!!Unknown Source!!";
        break;
    }

    switch (type) {
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        _type = "Deprecated Behavior";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        _type = "Undefined Behavior";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        _type = "Portability Issue";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        _type = "Performance Issue";
        break;
    case GL_DEBUG_TYPE_ERROR:
        _type = "General Error";
        break;
    case GL_DEBUG_TYPE_OTHER:
        _type = "Other Error";
        break;
    case GL_DEBUG_TYPE_MARKER:
        _type = "Debug Marker";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        _type = "Push Group";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        _type = "Pop Group";
        break;
    default:
        _type = "!!Unknown Error!!";
        break;
    }

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        _severity = "High";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        _severity = "Medium";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        _severity = "Low";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        _severity = "Notification";
        break;
    default:
        _severity = "!!Unknown Severity Level!!";
        break;
    }

    std::println(stderr, "({}) \tSeverity: {}\n\tIssue: {}\n\tLocation: {}\n\tDescription: {}",
                 id, _severity, _type, _src, _msg);

    std::fflush(stderr); // make sure error is visible in console before stopping
    __debugbreak();
}

void GLFWErrorCallback(int errCode, const char* errDesc) {
    std::println(stderr, "({}): {}", errCode, errDesc);
}
