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
//#define NOPRINT

namespace debug::cexpr {

constexpr void RedirectOutput(const char* filePathCStr, const char* errPathCStr = "") {
	if consteval {
		NOP;
	}
	else {
#ifdef _MSC_VER
		FILE *out1, *out2;
		freopen_s(&out1, filePathCStr, "w", stdout);

		if (errPathCStr && errPathCStr != "")
			freopen_s(&out2, errPathCStr, "w", stderr);
#else
		freopen(filePathCStr, "w", stdout);

		if (errPathCStr && errPathCStr != "")
			freopen(errPathCStr, "w", stderr);
#endif
	}
}

constexpr void exit(int exit_code) {
	if consteval {
		NOP;
	}
	else {
		std::exit(exit_code);
	}
}

template <class... Args>
constexpr void print(std::format_string<Args...> fmt, Args&&... args) {
	if consteval {
		NOP;
	}
#ifndef NOPRINT
	else {
		std::print(fmt, std::forward<Args>(args)...);
	}
#endif
}

template <class... Args>
constexpr void print(std::FILE* stream, std::format_string<Args...> fmt, Args&&... args) {
	if consteval {
		NOP;
	}
#ifndef NOPRINT
	else {
		std::print(stream, fmt, std::forward<Args>(args)...);
	}
#endif
}

template <class... Args>
constexpr void println(std::format_string<Args...> fmt, Args&&... args) {
	if consteval {
		NOP;
	}
#ifndef NOPRINT
	else {
		std::println(fmt, std::forward<Args>(args)...);
	}
#endif
}

template <class... Args>
constexpr void println(std::FILE* stream, std::format_string<Args...> fmt, Args&&... args) {
	if consteval {
		NOP;
	}
#ifndef NOPRINT
	else {
		std::println(stream, fmt, std::forward<Args>(args)...);
	}
#endif
}

// ignore disabling printing
template <class... Args>
constexpr void forceprinterr(std::format_string<Args...> fmt, Args&&... args) {
	if consteval {
		NOP;
	}
	else {
		std::print(stderr, fmt, std::forward<Args>(args)...);
	}
}

} // namespace debug::cexpr

#undef NOP
