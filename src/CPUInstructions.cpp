#include <eternal.hpp>
#include <array>

#ifdef DEBUG
#include <cstdlib>
#include <cassert>
#include <print>
#include <source_location>
#endif

#include "CPU.hpp"

namespace gb::cpu {

using InstrFunc = void(*)(Context&, Memory&);
using Addr16Getter = void(Context::RegisterFile::*)();
using Addr16Setter = void(Context::RegisterFile::*)(u16);

#define INSTR static void
#define INSTRMAP(x) { OpCode::##x, &x }

#ifdef DEBUG
[[noreturn]] static void NoImpl(std::source_location loc = std::source_location::current()) {
	std::println(stderr, "Unimplemented op code handler: {}", loc.function_name());
	std::exit(EXIT_FAILURE);
}
#endif

#pragma region shorthand functions
// Reads one byte from the memory, increments the program counter
// and adds an mcycle.
static byte Read(Context& cpu, Memory& mem) {
	cpu.MCycle();
	return mem[++cpu.reg.pc];
}

// Reads two bytes from memory and returns the data as a 16-bit value.
// Increments program counter twice and adds two mcycles.
static u16 Read2(Context& cpu, Memory& mem) {
	byte lo = mem[cpu.reg.pc++];
	cpu.MCycle();

	byte hi = mem[cpu.reg.pc++];
	cpu.MCycle();

	return static_cast<u16>(hi) << 8 | lo;
}

// Transform value (0-7) into an 8-bit register for use.
static byte& R8_FromBits(Context::RegisterFile& regs, Memory& mem, byte val) {
	assert(val < 8);

	switch (val) {
	case 0: return regs.b;
	case 1: return regs.c;
	case 2: return regs.d;
	case 3: return regs.e;
	case 4: return regs.h;
	case 5: return regs.l;
	case 6: return mem[regs.hl()]; // load byte stored in the location pointed to by hl
	case 7: return regs.a;
	default: std::unreachable();
	}
}

// Transform value (0-3) into a function pointer to a 16-bit register setter for use.
static Addr16Setter R16_SetFromBits(byte val) {
	assert(val < 4);
	using rf = Context::RegisterFile;

	switch (val) {
	case 0: return &rf::bc;
	case 1: return &rf::de;
	case 2: return &rf::hl;
	case 3: return &rf::spSet;
	default: std::unreachable();
	}
}

// Transforms a value (0-3) into a function pointer to a 16-bit register getter
static Addr16Getter R16_GetFromBits(byte val) {
	assert(val < 4);
	using rf = Context::RegisterFile;

	switch (val) {
	case 0: return static_cast<Addr16Getter>(rf::bc);
	default: std::unreachable();
	}
}

static Addr16Getter R16Stk_GetFromBits(byte val) {

}

static Addr16Getter R16Mem_GetFromBits(byte val) {

}

#pragma endregion

#pragma region non-prefixed instructions
	// Declared here so it can be stored in the map,
	// definition can be found in the prefixed instructions section
INSTR cb_prefix(Context&, Memory&);

INSTR nop(Context& cpu, Memory& mem) {
	// TODO: m cycle?
}

INSTR ld_r8_r8(Context& cpu, Memory& mem) {
	NoImpl();
}

INSTR jp_imm16(Context& cpu, Memory& mem) {
	u16 addr = Read2(cpu, mem);

	cpu.reg.pc = addr;
	cpu.MCycle();
}
#pragma endregion
#pragma region prefixed (cb) instructions
INSTR cb_prefix(Context& cpu) {

}
#pragma endregion

static constexpr auto instrs = mapbox::eternal::map<OpCode, InstrFunc>(
{
	INSTRMAP(nop),
	INSTRMAP(ld_r8_r8),
	INSTRMAP(jp_imm16)
});

//static constexpr auto cbInstrs = mapbox::eternal::map<OpCode, InstrFunc>(
//{
//
//});

#undef INSTRMAP
#undef INSTR

static constexpr auto InvalidInstrs = std::to_array<byte>(
{
	0xD3, 0xDB, 0xDD, 0xE3, 0xE4, 0xEB, 0xEC, 0xED, 0xF4, 0xFC, 0xFD
});

bool Context::Fetch() {
	byte nextOp = _memory[reg.pc++];
	MCycle();

	if (std::ranges::contains(InvalidInstrs, nextOp)) {
		// TODO: hang cpu
		std::println(stderr, "CPU should hang");
		return false;
	}

	ir = nextOp;

	return true;
}

bool Context::Exec() {
	auto opCode = instrs.find(static_cast<OpCode>(ir));

	if (opCode == instrs.end()) {
		std::println(stderr, "Unhandled op code: {:#04x}", ir);
		return false;
	}

	InstrFunc handler = opCode->second;
	handler(*this, _memory);

	return true;
}

} // namespace gb::cpu
