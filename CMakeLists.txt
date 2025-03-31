cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

project(GameboyEmulator)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)

option(ENABLE_TESTS "Enable gameboy emulator tests" ON)

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

FetchContent_MakeAvailable(glad glfw)

glad_add_library(glad STATIC API gl:core=4.6)

add_subdirectory(src/imgui)

include_directories(${PROJECT_NAME} PUBLIC include include/imgui)

add_executable(${PROJECT_NAME} ${IMGUISRC} "src/main.cpp" "src/ROM.cpp" "src/CPU.cpp" "src/Memory.cpp" "src/CPUInstructions.cpp")

target_compile_definitions(${PROJECT_NAME}
    PRIVATE "$<$<CONFIG:Debug,RelWithDebInfo>:DEBUG>"
    PUBLIC ROMPATH="${CMAKE_CURRENT_SOURCE_DIR}/roms"
    PRIVATE "$<$<CONFIG:Debug,RelWithDebInfo>:$<$<BOOL:${ENABLE_TESTS}>:TESTS>>"
    PRIVATE TESTPATH="$<$<CONFIG:Debug,RelWithDebInfo>:$<$<BOOL:${ENABLE_TESTS}>:${CMAKE_CURRENT_SOURCE_DIR}/tests>>"
)

if (MSVC)
    target_compile_options(${PROJECT_NAME} PUBLIC /Zi)
    target_link_options(${PROJECT_NAME} PUBLIC /INCREMENTAL)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC include ${GG_LOCATION} include/external)

# i couldn't get eternal to work as a library
target_link_libraries(${PROJECT_NAME} PRIVATE
  ${OPENGL_LIBRARIES}
  glad
  glfw
)
