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

using Addr16Getter = u16(Context::RegisterFile::*)() const;
using Addr16Setter = void(Context::RegisterFile::*)(u16);

#define INSTR static void

#ifdef DEBUG
static std::string_view GetFunctionName(const std::source_location& loc) {
	std::string_view name = loc.function_name();
	auto start = name.find("gb::cpu::");
	auto end = name.find('(');

	return name.substr(name.find("gb::cpu::") + 9, end - start - 9);
}

[[noreturn]] static void NoImpl(std::source_location loc = std::source_location::current())
{
	auto name = GetFunctionName(loc);
	std::println(stderr, "Unimplemented op code handler: {}", name);
	std::exit(EXIT_FAILURE);
}

static void PrintFuncName(std::source_location loc = std::source_location::current()) {
	auto name = GetFunctionName(loc);
	std::println("Function: {}", name);
}

#define NOIMPL() NoImpl()
#define PRINTFUNC() PrintFuncName()
#else
#define NOIMPL() (void*)0
#define PRINTFUNC() (void*)0
#endif

#pragma region shorthand functions
// Reads one byte from the memory, increments the program counter
// and adds an mcycle.
static byte Read(Context& cpu, Memory& mem) {
	cpu.MCycle();
	return mem[cpu.reg.pc++];
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
	case 0: return &rf::bc;
	case 1: return &rf::de;
	case 2: return &rf::hl;
	case 3: return &rf::spGet;
	default: std::unreachable();
	}
}

// Transforms a value (0-3) into a function pointer to a 16-bit register getter (stack).
static Addr16Getter R16Stk_GetFromBits(byte val) {
	assert(val < 4);
	using rf = Context::RegisterFile;

	switch (val) {
	case 0: return &rf::bc;
	case 1: return &rf::de;
	case 2: return &rf::hl;
	case 3: return &rf::af;
	default: std::unreachable();
	}
}

// Transforms a value (0-3) into a function pointer to a 16-bit register getter (memory).
static Addr16Setter R16Mem_GetFromBits(byte val) {
	assert(val < 4);
	using rf = Context::RegisterFile;

	switch (val) {
	case 0: return &rf::bc;
	case 1: return &rf::de;
	case 2: return &rf::hlPlus;
	case 3: return &rf::hlMinus;
	default: std::unreachable();
	}
}

#pragma endregion

#pragma region non-prefixed instructions
INSTR nop(Context& cpu, Memory& mem) {
	// TODO: m cycle?
	PRINTFUNC();
}

INSTR ld_r8_r8(Context& cpu, Memory& mem) {
	PRINTFUNC();
	/*
	* ld [hl], [hl] would normally halt the cpu. it isn't handled here
	* because ld [hl], [hl] directly corresponds to the halt instruction.
	* non-variable instructions are handled before this function would even have
	* a chance to run
	*/

	byte destVal = (cpu.ir & 0b00'111'000) >> 3;
	byte srcVal = cpu.ir & 0b00000'111;

	byte& dest = R8_FromBits(cpu.reg, mem, destVal);
	byte& src = R8_FromBits(cpu.reg, mem, srcVal);

	dest = src;

	cpu.MCycle();
}

INSTR ld_r8_imm8(Context& cpu, Memory& mem) {
	PRINTFUNC();

	byte destVal = (cpu.ir & 0b00'111'000) >> 3;
	byte& reg = R8_FromBits(cpu.reg, mem, destVal);

	reg = Read(cpu, mem);
	cpu.MCycle();
}

INSTR ld_acc_r16mem(Context& cpu, Memory& mem) {
	PRINTFUNC();

	byte destVal = (cpu.ir & 0b00'11'0000) >> 4;
	Addr16Getter handle = R16_GetFromBits(destVal);

	byte data = mem[(cpu.reg.*handle)()];
	cpu.MCycle();

	cpu.reg.a = data;
	cpu.MCycle();
}

INSTR ld_r16_imm16(Context& cpu, Memory& mem) {
	PRINTFUNC();

	u16 data = Read2(cpu, mem);

	byte destVal = (cpu.ir & 0b00'11'0000) >> 4;
	Addr16Setter handle = R16_SetFromBits(destVal);
	
	(cpu.reg.*handle)(data);
	cpu.MCycle();
}

INSTR jp_imm16(Context& cpu, Memory& mem) {
	PRINTFUNC();

	u16 addr = Read2(cpu, mem);

	cpu.reg.pc = addr;
	cpu.MCycle();
}
#pragma endregion

#pragma region prefixed (cb) instructions
INSTR cb_prefix(Context& cpu, Memory&) {
	NOIMPL();
}
#pragma endregion

#undef INSTR

using InstrPair = std::pair<OpCode, Context::InstrFunc>; // shorthand
#define INSTRMAP(x) { OpCode::##x, &x }

// Contains the mapping of op code -> handler.
// ignoreBits sets each bit that should effectively be ignored when looking up an op code.
struct VariableInstrData {
	Context::InstrFunc handler;
	OpCode op;
	byte ignoreBits; 
};
#define INSTRDATA(op, byte) { &op, OpCode::##op, byte }


// A mapping of all the instructions that do not have multiple different possible op codes
// such as ld_r8_r8, where 6 bits can differ.
static constexpr auto constInstrMap = mapbox::eternal::map<OpCode, Context::InstrFunc>(
{
	INSTRMAP(nop),
	INSTRMAP(jp_imm16),
	INSTRMAP(cb_prefix)
});

// A mapping of all the instructions that have many possible op codes
// Stored as an array because find_if is used on this collection, which removes any benefit
// of using a map.
static constexpr auto variableInstrMap = std::to_array<VariableInstrData>(
{
	INSTRDATA(ld_r8_r8, 0b00'111'111),
	INSTRDATA(ld_r16_imm16, 0b00'11'0000),
	INSTRDATA(ld_r8_imm8, 0b00'111'000),
	INSTRDATA(ld_acc_r16mem, 0b00'11'0000)
});

// A mapping of all the cb instructions that have many possible op codes.
// Stored as an array for the same reason as variableInstrMap
//static constexpr auto cbInstrMap = std::to_array<VariableInstrData>(
//{
//
//});

#undef INSTRMAP

static constexpr auto InvalidInstrs = std::to_array<byte>(
{
	0xD3, 0xDB, 0xDD, 0xE3, 0xE4, 0xEB, 0xEC, 0xED, 0xF4, 0xFC, 0xFD
});

bool Context::Fetch() {
	namespace rng = std::ranges;

	ir = _memory[reg.pc++];
	MCycle();

	OpCode opCode = static_cast<OpCode>(ir);

	// Make sure it's not a undefined op code
	if (rng::contains(InvalidInstrs, ir)) {
		// TODO: hang cpu
#ifdef DEBUG
		std::println(stderr, "CPU should hang");
#endif

		_handler = nullptr;
		return false;
	}

	// First, check if it's a non-variable instruction
	if (auto it = constInstrMap.find(opCode); it != constInstrMap.end()) {
#ifdef DEBUG
		std::println("Op Code (ir): {:#010b} ({:#04x})\tFound: {:#010b} ({:#04x})",
					 ir, ir, static_cast<byte>(it->first), static_cast<byte>(it->first));
#endif

		_handler = it->second;
		return true;
	}

	// If not found, it's a variable instruction.
	auto it = rng::find_if(
		variableInstrMap,
		[this](const VariableInstrData& data) {
			byte opAsByte = static_cast<byte>(data.op);
			byte irWithIgnore = ir & ~data.ignoreBits;

			/* ex:		ir == 0b00'01'0001
			*			op == 0b00'00'0001
			*	ignoreBits == 0b00'11'0000 (2s complement: 0b11'00'1111)
			* irWithIgnore == 0b00'00'0001
			*		   res == 0b00'00'0001
			* res == op <-- instruction found
			*/
			return irWithIgnore == opAsByte;
		}
	);

	// Couldn't find a valid instruction, something went wrong.
	if (it == variableInstrMap.end()) {
#ifdef DEBUG
		std::print(stderr, "Couldn't find instruction! ");
		std::println(stderr, "Op Code (ir): {:#010b} ({:#04x})", ir, ir);
#endif

		_handler = nullptr;
		return false;
	}

#ifdef DEBUG
	std::println("Op Code (ir): {:#010b} ({:#04x})\tFound: {:#010b} ({:#04x})",
				 ir, ir, static_cast<byte>(it->op), static_cast<byte>(it->op));
#endif
	// Store a function pointer rather than updating the ir
	// because some instructions have variable op codes. The ir still holds the bits
	// to determine what register / data / condition needs to be used.
	_handler = it->handler;

	return true;
}

bool Context::Exec() {
	if (!_handler) {
#ifdef DEBUG
		std::println("Invalid instruction!");
#endif

		return false;
	}

	_handler(*this, _memory);

	return true;
}

} // namespace gb::cpu
