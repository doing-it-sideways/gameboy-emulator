#pragma once

#include <bit>
#include <cstdint>

#if defined(DEBUG) && defined(__cpp_lib_debugging)
#include <debugging>
#endif

namespace gb {

using u8 = std::uint8_t;
using u16 = std::uint16_t;

using byte = u8;
using u64 = std::uint64_t;

}

#ifdef DEBUG

#ifdef __cpp_lib_debugging // c++26
#define BREAKPOINT std::breakpoint_if_debugging()
#elif defined(_MSC_VER)
#define BREAKPOINT __debugbreak()
#elif defined(__clang__)
#define BREAKPOINT __builtin_debugtrap()
#elif defined(__GNUC__) || defined(__GNUG__)
#define BREAKPOINT __builtin_trap()
#endif // breakpoint definitions

#endif // DEBUG
