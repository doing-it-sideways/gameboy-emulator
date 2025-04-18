cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

project(GameboyEmulator)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)

option(ENABLE_TESTS "Enable gameboy emulator tests" ON)

set(WITH_TESTS OFF CACHE BOOL "broken option thanks :thumbs_up:" FORCE)
add_subdirectory(external/eternal)

add_subdirectory(emulator)
add_subdirectory(emulator-testing)
