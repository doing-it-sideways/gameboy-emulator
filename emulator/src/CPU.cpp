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
	, _memory(memory)
{
	// TODO
}

bool Context::Update() {
	if (_isHalted) {
		MCycle();

		auto [ie, ieReq] = _memory.GetInterruptReg();
		// cpu wakes when bitwise and of ie and if != 0
		// TODO: implement halt bug (gbdev 9.2)
		if (ie & ieReq)
			_isHalted = false;
	}
	else {
#if defined(DEBUG)
		if (canDump)
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

	if (_ime) {
		InterruptHandler();
	}
	else if (_enablingIME) {
		_ime = true;
		_enablingIME = false;
	}

	return true;
}

void Context::InterruptHandler() {
	MCycle(1); // simulate "2" nops. second nop happens at top of PushStack
	PushStack(reg.pc);

	// TODO: NMI (0x80) is 2nd highest priority after bugged interrupt (0x00)
	auto [ie, iF] = _memory.GetInterruptReg();
	for (byte i = 0, interrupt = 1; i < 5; ++i, interrupt <<= 1) {
		if (ie & interrupt && iF & interrupt) {
			_ime = false;

			reg.pc = 0x40 + (i * 8);
			iF = static_cast<byte>(iF) & ~interrupt;

			MCycle();

			return; // only handle one interrupt at a time
		}
	}
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
	MCycle(); // sp - 1

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

void Context::Halt() {
	_isHalted = true;
	--reg.pc; // IR doesn't increment on halt
}

void Context::Hang() {
	// TODO: hang cpu
	debug::cexpr::println(stderr, "CPU should hang");

	_handler = nullptr;
	BREAKPOINT;
}

void Context::EnableInterrupts() {
	_enablingIME = true;
}

void Context::DisableInterrupts() {
	_enablingIME = false;
	_ime = false;
}

void Context::Dump() const {
	byte data = _memory[reg.pc];
	auto [ie, _] = _memory.GetInterruptReg();

	debug::cexpr::println("\n---Current CPU State---");
	debug::cexpr::println("pc ({:#06x}): {:#04x}", reg.pc, data);
	debug::cexpr::println("flags: {:04b}", reg.f >> 4);
	debug::cexpr::println("registers: a: {:#04x}, bc: {:#06x}, de: {:#06x}, hl: {:#06x}",
				 reg.a, reg.bc(), reg.de(), reg.hl());
	debug::cexpr::println("ir: {:#010b}\tie: {:#010b}\n", ir, static_cast<byte>(ie));
}

} // namespace gb::cpu