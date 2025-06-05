set(EMU_LIB gbemu)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)

add_library(${EMU_LIB} STATIC "src/ROM.cpp" "src/CPU.cpp" "src/Memory.cpp" "src/CPUInstructions.cpp"  "src/MapperChipInfo.cpp" "src/Screen.cpp" "src/Emulator.cpp" "src/PPU.cpp" "src/HardwareRegisters.cpp" "src/Timer.cpp")

target_compile_definitions(${EMU_LIB}
    PUBLIC "$<$<CONFIG:Debug,RelWithDebInfo>:DEBUG>"
    PUBLIC "$<$<CONFIG:Debug,RelWithDebInfo>:$<$<BOOL:${ENABLE_TESTS}>:TESTS>>"
    PUBLIC TESTPATH="$<$<CONFIG:Debug,RelWithDebInfo>:$<$<BOOL:${ENABLE_TESTS}>:${PROJECT_SOURCE_DIR}/tests>>"
)

target_include_directories(${EMU_LIB}
    PUBLIC include
    PRIVATE ../external/eternal/include
)

target_link_libraries(${EMU_LIB} PUBLIC graphicslib)

if (MSVC)
    target_compile_options(${EMU_LIB} PUBLIC /Zi)
    target_link_options(${EMU_LIB} PUBLIC /INCREMENTAL)
endif()
