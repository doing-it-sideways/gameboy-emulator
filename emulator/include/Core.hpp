#pragma once

#include <bit>
#include <cstdint>
#include <cassert>

#if defined(DEBUG) && defined(__cpp_lib_debugging)
#include <debugging>
#endif

namespace gb {

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using s8 = std::int8_t;

using byte = u8;
using sbyte = s8;

static constexpr u16 interruptEnd = 0x0100;	// restarts and interrupts [$0000, $0FFF]
static constexpr u16 headerEnd = 0x0150;	// cartridge header [$0100, $014F]
static constexpr u16 rom0End = 0x4000;		// fixed rom bank [$0150, $3FFF]
static constexpr u16 romNEnd = 0x8000;		// switchable rom bank [$4000, $7FFF]
static constexpr u16 vramEnd = 0xA000;		// video ram [$8000, $9FFF]
static constexpr u16 ramCartEnd = 0xC000;	// cartridge ram [$A000, $BFFF]
static constexpr u16 ram0End = 0xD000;		// internal gameboy ram [$C000, $CFFF]
static constexpr u16 ramNEnd = 0xE000;		// internal gameboy ram [$D000, $DFFF]
static constexpr u16 echoRamEnd = 0xFE00;	// Prohibited to use. [$E000, $FDFF]
static constexpr u16 oamEnd = 0xFEA0;		// object attribute memory [$FE00, $FE9F]
static constexpr u16 unusableEnd = 0xFF00;	// Prohibited to use. [$FEA0, $FEFF]
static constexpr u16 ioEnd = 0xFF80;		// io registers [$FF00, $FF7F]
static constexpr u16 hramEnd = 0xFFFF;		// high ram (Zero page) [$FF80, $FFFE]
static constexpr u16 regIE = 0xFFFF;		// Interrupt enable register ($FFFF)

static constexpr u16 romBankSize = 1 << 14;
static constexpr u16 ramBankSize = 1 << 12;

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
#endif // __cpp_lib_debugging -- breakpoint definitions

#endif // DEBUG
