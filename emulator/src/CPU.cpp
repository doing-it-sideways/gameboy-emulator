#include "CPU.hpp"
#include "ConstexprAdditions.hpp"
#include "Memory.hpp"

namespace gb::cpu {

// https://gbdev.io/pandocs/Power_Up_Sequence.html
// DMG -- todo?: allow for different cpus: DMG0, MGB, maybe CGB support later?
Context::Context(Memory& memory)
	: reg{ .pc = 0x0100, .sp = 0xFFFE,
		   .a = 0x01, .f = { 1, 0, 1, 1 },
		   .c = 0x13, .d = 0, .e = 0xD8, .h = 0x01, .l = 0x4D }
	, ir(memory[0x0100])
	, ie(0x00)
	, _memory(memory)
{
	// TODO
}

bool Context::Update() {
	if (_isHalted) {
		// TODO: handle halt and resumtion to normal execution
	}
	else {
#if defined(DEBUG) && defined(TESTS)
		Dump(); // print current state of cpu
#endif

		// fetch and execute overlap on the SM83.
		if (!Fetch()) {
			// TODO
			return false;
		}

		if (!Exec()) {
			// TODO
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

void Context::PushStack(u16 value) {
	_memory[--reg.sp] = (value & 0xFF00) >> 8;
	MCycle();

	_memory[--reg.sp] = value & 0x00FF;
	MCycle();
}

u16 Context::PopStack() {
	const byte lo = _memory[reg.sp++];
	MCycle();

	const byte hi = _memory[reg.sp++];
	MCycle();

	return (hi << 8) | lo;
}

void Context::MCycle(u8 cycles) {
	// TODO
	_mCycles += cycles;
}

void Context::Hang() {
	// TODO: hang cpu
	debug::cexpr::println(stderr, "CPU should hang");

	_handler = nullptr;
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