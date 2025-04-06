#pragma once

#include <print>
#include <type_traits>

/*
* This entire file provides constexpr wrappers for non-constexpr functions/macros
* that are a part of the standard library. This essentially turns most functions into
* no-ops during compile time execution. Function signatures will
* attempt to exactly match what is in the standard.
*/

#define NOP (void)0

//#define CExprAssert(x) { \
//	if (std::is_constant_evaluated()) assert(x); \
//	else (void)0; \
//}

namespace debug::cexpr {

constexpr void nop() noexcept { (void)0; }

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
		std::print(fmt, std::forward<Args>(args)...);
}

template <class... Args>
constexpr void print(std::FILE* stream, std::format_string<Args...> fmt, Args&&... args) {
	if (std::is_constant_evaluated())
		NOP;
	else
		std::print(stream, fmt, std::forward<Args>(args)...);
}

template <class... Args>
constexpr void println(std::format_string<Args...> fmt, Args&&... args) {
	if (std::is_constant_evaluated())
		NOP;
	else
		std::println(fmt, std::forward<Args>(args)...);
}

template <class... Args>
constexpr void println(std::FILE* stream, std::format_string<Args...> fmt, Args&&... args) {
	if (std::is_constant_evaluated())
		NOP;
	else
		std::println(stream, fmt, std::forward<Args>(args)...);
}

} // namespace debug::cexpr

#undef NOP
