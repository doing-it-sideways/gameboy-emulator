#pragma once

#ifdef DEBUG
#include <cassert>
#include <print>
#include <type_traits>
#endif

/*
* This entire file provides constexpr wrappers for non-constexpr functions/macros
* that are a part of the standard library. This essentially turns most functions into
* no-ops during compile time execution. Additionally, it will also disable functions
* during release builds by turning them into no-ops as well. Function signatures will
* attempt to exactly match what is in the standard.
*/

#define NOP (void)0

//#define CExprAssert(x) { \
//	if (std::is_constant_evaluated()) assert(x); \
//	else (void)0; \
//}

namespace debug::cexpr {

constexpr void nop() noexcept { (void)0; }

#ifdef DEBUG
constexpr void exit(int exit_code) {
	if (std::is_constant_evaluated())
		NOP;
	else
		std::exit(exit_code);
}

template <class... Args>
constexpr void print(std::format_string<Args...> fmt, Args&&... args) {
	if (std::is_constant_evaluated())
		NOP;
	else
		std::print(fmt, args...);
}

template <class... Args>
constexpr void print(std::FILE* stream, std::format_string<Args...> fmt, Args&&... args) {
	if (std::is_constant_evaluated())
		NOP;
	else
		std::print(stream, fmt, args...);
}

template <class... Args>
constexpr void println(std::format_string<Args...> fmt, Args&&... args) {
	if (std::is_constant_evaluated())
		NOP;
	else
		std::println(fmt, args...);
}

template <class... Args>
constexpr void println(std::FILE* stream, std::format_string<Args...> fmt, Args&&... args) {
	if (std::is_constant_evaluated())
		NOP;
	else
		std::println(stream, fmt, args...);
}
#else
#define exit(exit_code) nop()
#define print(fmt, ...) nop()
#define print(stream, fmt, ...) nop()
#define println(fmt, ...) nop()
#define println(stream, fmt, ...) nop()
#endif

} // namespace debug::cexpr

#undef NOP
