#include "CPU.hpp"

#ifdef DEBUG
#include <print>
#endif

namespace gb::cpu {

// https://gbdev.io/pandocs/Power_Up_Sequence.html
// DMG -- todo?: allow for different cpus: DMG0, MGB, maybe CGB support later?
constexpr Context::Context()
	: reg{ .pc = 0x0100, .sp = 0xFFFE,
	       .a = 0x01, .f = 0b10110000, .c = 0x13, .d = 0, .e = 0xD8, .h = 0x01, .l = 0x4D }
	, ir(0x00)
	, ie(0x00)
{
	// TODO
}

constexpr void Context::Inc(u16 reg) {
	u16 val;
	if (val = bc(); reg == val)
		bc(val + 1);
	else if (val = de(); reg == val)
		de(val + 1);
	else if (val = hl(); reg == val)
		hl(val + 1);
#ifdef DEBUG
	else
		std::println(stderr, "Unknown register!");
#endif
}

constexpr void Context::Dec(u16 reg) {
	u16 val;
	if (val = bc(); reg == val)
		bc(val - 1);
	else if (val = de(); reg == val)
		de(val - 1);
	else if (val = hl(); reg == val)
		hl(val - 1);
#ifdef DEBUG
	else
		std::println(stderr, "Unknown register!");
#endif
}

} // namespace gb::cpu