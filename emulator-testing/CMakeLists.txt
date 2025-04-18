set(TEST_PROGRAM gbtests)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)

add_executable(${TEST_PROGRAM} main.cpp)

target_include_directories(${TEST_PROGRAM} PUBLIC
    ${PROJECT_SOURCE_DIR}/emulator/include
    ${PROJECT_SOURCE_DIR}/graphics/include
)

target_link_libraries(${TEST_PROGRAM}
	PRIVATE gbemu
	PRIVATE graphicslib
)

if (MSVC)
    target_compile_options(${TEST_PROGRAM} PUBLIC /Zi)
    target_link_options(${TEST_PROGRAM} PUBLIC /INCREMENTAL)
endif()
