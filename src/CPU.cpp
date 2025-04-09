#include "CPU.hpp"
#include "ConstexprAdditions.hpp"

namespace gb::cpu {

// https://gbdev.io/pandocs/Power_Up_Sequence.html
// DMG -- todo?: allow for different cpus: DMG0, MGB, maybe CGB support later?
Context::Context(rom::RomData&& romData)
	: reg{ .pc = 0x0100, .sp = 0xFFFE,
		   .a = 0x01, .f = { 1, 0, 1, 1 },
		   .c = 0x13, .d = 0, .e = 0xD8, .h = 0x01, .l = 0x4D }
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
			debug::cexpr::println(stderr, "Something went wrong!");
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
		debug::cexpr::println(stderr, "Unknown register!");
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
		debug::cexpr::println(stderr, "Unknown register!");
		BREAKPOINT;
	}
#endif // DEBUG
}

void Context::Dump() const {
	debug::cexpr::println("\n---Current CPU State---");
	byte data = _memory[reg.pc];
	debug::cexpr::println("pc ({:#06x}): {:#04x}", reg.pc, data);
	debug::cexpr::println("flags: {:04b}", reg.f >> 4);
	debug::cexpr::println("registers: a: {:#04x}, bc: {:#06x}, de: {:#06x}, hl: {:#06x}",
				 reg.a, reg.bc(), reg.de(), reg.hl());
	debug::cexpr::println("ir: {:#010b}\tie: {:#010b}\n", ir, ie);
}

} // namespace gb::cpu