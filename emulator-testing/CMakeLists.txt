set(TEST_PROGRAM gbtests)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)

add_executable(${TEST_PROGRAM} main.cpp)

target_link_libraries(${TEST_PROGRAM}
	PRIVATE gbemu
)

if (MSVC)
    target_compile_options(${TEST_PROGRAM} PUBLIC /Zi)
    target_link_options(${TEST_PROGRAM} PUBLIC /INCREMENTAL)
    set_target_properties(${TEST_PROGRAM} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/")
endif()
