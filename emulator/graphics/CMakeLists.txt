set(GRAPHICS_LIB graphicslib)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)

find_package(OpenGL REQUIRED)
if (NOT OpenGL_FOUND)
    message(FATAL_ERROR "No OpenGL detected")
endif()

include_directories(${OPENGL_INCLUDE_DIR})

include(FetchContent)

function(DeclareGitLibrary LIBNAME REPOLINK REPOTAG)
    FetchContent_Declare(
        ${LIBNAME}
        GIT_REPOSITORY ${REPOLINK}
        GIT_TAG ${REPOTAG}
        GIT_SHALLOW true
    )
endfunction()

FetchContent_Declare(glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.8
    GIT_SHALLOW true
    SOURCE_SUBDIR cmake
)

DeclareGitLibrary(glfw https://github.com/glfw/glfw.git 3.4)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

DeclareGitLibrary(glm https://github.com/g-truc/glm.git bf71a834948186f4097caa076cd2663c69a10e1e)
set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
set(GLM_ENABLE_CXX_20 ON CACHE BOOL "" FORCE)

if (ENABLE_GLM_AVX2)
    set(GLM_ENABLE_SIMD_AVX2 ON CACHE BOOL "" FORCE)
endif(ENABLE_GLM_AVX2)
unset(ENABLE_GLM_AVX2 CACHE)

FetchContent_MakeAvailable(glad glfw glm)

glad_add_library(glad STATIC API gl:core=4.6)

add_subdirectory(src/imgui)

add_library(${GRAPHICS_LIB} STATIC ${IMGUISRC} "src/Error.cpp" "src/ShaderParser.cpp" "src/Shader.cpp" "src/Texture.cpp" "src/GLBuffers.cpp")

target_include_directories(${GRAPHICS_LIB} PUBLIC ${GG_LOCATION} include include/imgui)

target_link_libraries(${GRAPHICS_LIB} PRIVATE
  ${OPENGL_LIBRARIES}
  glad
  glfw
  glm::glm
)
