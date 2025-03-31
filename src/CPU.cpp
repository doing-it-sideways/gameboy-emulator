#include "CPU.hpp"

#ifdef DEBUG
#include <print>
#endif

namespace gb::cpu {

// https://gbdev.io/pandocs/Power_Up_Sequence.html
// DMG -- todo?: allow for different cpus: DMG0, MGB, maybe CGB support later?
Context::Context(rom::RomData&& romData)
	: reg{ .pc = 0x0100, .sp = 0xFFFE,
	       .a = 0x01, .f = 0b10110000, .c = 0x13, .d = 0, .e = 0xD8, .h = 0x01, .l = 0x4D }
	, ir(romData[0x0100])
	, ie(0x00)
	, _memory(std::move(romData))
{
	// TODO
}

void Context::Run() {
	_isRunning = true;

	while (_isRunning) {
		if (_isPaused) {
			// TODO: add delay
			continue;
		}

		if (!Update()) {
			std::println(stderr, "Something went wrong!");
			return;
		}
	}
}

bool Context::Update() {
	if (_isHalted) {
		// TODO: handle halt and resumtion to normal execution
	}
	else {
#ifdef DEBUG
		Dump(); // print current state of cpu
#endif

		// fetch and execute overlap on the SM83.
		if (!Fetch()) {
			// TODO
			_isRunning = false;
			return false;
		}

		if (!Exec()) {
			// TODO
			_isRunning = false;
			return false;
		}
	}

	return true;
}

void Context::Inc(u16 reg16) {
	u16 val;
	if (val = reg.bc(); reg16 == val)
		reg.bc(val + 1);
	else if (val = reg.de(); reg16 == val)
		reg.de(val + 1);
	else if (val = reg.hl(); reg16 == val)
		reg.hl(val + 1);
#ifdef DEBUG
	else {
		std::println(stderr, "Unknown register!");
		BREAKPOINT;
	}
#endif
}

void Context::Dec(u16 reg16) {
	u16 val;
	if (val = reg.bc(); reg16 == val)
		reg.bc(val - 1);
	else if (val = reg.de(); reg16 == val)
		reg.de(val - 1);
	else if (val = reg.hl(); reg16 == val)
		reg.hl(val - 1);
#ifdef DEBUG
	else {
		std::println(stderr, "Unknown register!");
		BREAKPOINT;
	}
#endif
}

#ifdef DEBUG
void Context::Dump() const {
	std::println("\n---Current CPU State---");
	std::println("pc ({:#06x}): {:#04x}", reg.pc, _memory[reg.pc]);
	std::println("flags: {:04b}", reg.f >> 4);
	std::println("registers: a: {:#04x}, bc: {:#06x}, de: {:#06x}, hl: {:#06x}",
				 reg.a, reg.bc(), reg.de(), reg.hl());
	std::println("ir: {:#010b}\tie: {:#010b}\n", ir, ie);
}
#endif // DEBUG

} // namespace gb::cpu