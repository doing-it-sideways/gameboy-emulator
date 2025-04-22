#include <mapbox/eternal.hpp>
#include <array>

#ifdef DEBUG
#include <cstdlib>
#include <source_location>
#include "ConstexprAdditions.hpp"
#endif

#include "CPU.hpp"

namespace gb::cpu {

#pragma region debug functions
#ifdef DEBUG
constexpr static std::string_view GetFunctionName(const std::source_location& loc) {
	const std::string_view name = loc.function_name();
	const std::size_t start = name.find("gb::cpu::");
	const std::size_t end = name.find('(');

	return name.substr(start + 9, end - start - 9);
}

[[noreturn]]
constexpr static void NoImpl(std::source_location loc = std::source_location::current()) {
	const std::string_view name = GetFunctionName(loc);
	debug::cexpr::forceprinterr("Unimplemented op code handler: {}\n", name);
	debug::cexpr::exit(EXIT_FAILURE);
}

constexpr static void PrintFuncName(std::source_location loc = std::source_location::current()) {
	const std::string_view name = GetFunctionName(loc);
	debug::cexpr::println("Function: {}", name);
}

#define NOIMPL() NoImpl()
#define PRINTFUNC() PrintFuncName()
#else // DEBUG
#define NOIMPL() (void)0
#define PRINTFUNC() (void)0
#endif // DEBUG

#pragma endregion debug functions

using Addr16Getter = u16(Context::RegisterFile::*)() const;
using Addr16MemGetter = u16(Context::RegisterFile::*)();
using Addr16Setter = void(Context::RegisterFile::*)(u16);

#define INSTR static void

// Wrapper struct so that indirect hl access gets handled properly for 8-bit registers
struct R8Reg {
	Memory& mem;
	byte& reg;
	const bool isIndirectHL;

	constexpr R8Reg(Memory& memory, byte& val, bool isHL = false)
		: mem(memory)
		, reg(val)
		, isIndirectHL(isHL)
	{}

	constexpr R8Reg& operator=(byte data) {
		if (isIndirectHL)
			mem.Write(reg, data);
		else
			reg = data;

		return *this;
	}

	constexpr R8Reg& operator=(R8Reg& other) { return operator=(other.reg); }

	// postfix operators not supported
	constexpr R8Reg& operator++() { return operator=(reg + 1); }
	constexpr R8Reg& operator--() { return operator=(reg - 1); }

	constexpr bool operator==(byte val) { return reg == val; }

	constexpr byte operator&(byte val) { return reg & val; }
	constexpr byte operator|(byte val) { return reg | val; }
	constexpr byte operator^(byte val) { return reg ^ val; }
	constexpr byte operator<<(byte val) { return reg << val; }
	constexpr byte operator>>(byte val) { return reg >> val; }

	static friend constexpr byte operator&(byte a, R8Reg& b) { return a & b.reg; }
	static friend constexpr byte operator|(byte a, R8Reg& b) { return a | b.reg; }
	static friend constexpr byte operator^(byte a, R8Reg& b) { return a ^ b.reg; }
	static friend constexpr byte operator<<(byte a, R8Reg& b) { return a >> b.reg; }
	static friend constexpr byte operator>>(byte a, R8Reg& b) { return a << b.reg; }
};

#pragma region value retrieving functions
// Transform value [0, 7] into an 8-bit register for use.
static R8Reg R8_FromBits(Context::RegisterFile& regs, Memory& mem, byte val) {
	assert(val < 8);

	switch (val) {
	case 0: return { mem, regs.b };
	case 1: return { mem, regs.c };
	case 2: return { mem, regs.d };
	case 3: return { mem, regs.e };
	case 4: return { mem, regs.h };
	case 5: return { mem, regs.l };
		  
	// Load byte stored in the location pointed to by hl
	case 6: return { mem, mem[regs.hl()], true };

	case 7: return { mem, regs.a };
	default: std::unreachable();
	}
}

// Transform value [0, 3] into a function pointer to a 16-bit register setter for use.
constexpr static Addr16Setter R16_SetFromBits(byte val) {
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

// Transforms a value [0, 3] into a function pointer to a 16-bit register getter
constexpr static Addr16Getter R16_GetFromBits(byte val) {
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

// Transforms a value [0, 3] into a function pointer to a 16-bit register getter (memory).
constexpr static Addr16Setter R16Mem_SetFromBits(byte val) {
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

// Transforms a value [0, 3] into a function pointer to a 16-bit register getter (memory).
constexpr static Addr16MemGetter R16Mem_GetFromBits(byte val) {
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

// Transforms a value [0, 3] into a function pointer to a 16-bit register getter (stack).
constexpr static Addr16Getter R16Stk_GetFromBits(byte val) {
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

// Transforms a value [0, 3] into a check for a certain value in the flags.
static bool FlagCond(Context::Flags flags, byte val) {
	assert(val < 4);

	switch (val) {
	case 0: return !static_cast<bool>(flags.Zero);	// NZ
	case 1: return static_cast<bool>(flags.Zero);	// Z
	case 2: return !static_cast<bool>(flags.Carry);	// NC
	case 3: return static_cast<bool>(flags.Carry);	// C
	default: break;
	}

	debug::cexpr::println(stderr, "Unknown condition check: {:#04b}", val);
	return false;
}

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

	u16 hi = mem[cpu.reg.pc++];
	cpu.MCycle();

	return hi << 8 | lo;
}

#pragma endregion value retrieving functions

// Forward declared because it handles calling all cb prefixed instrs.
// Needed for putting it in the instruction map.
INSTR cb_prefix(Context& cpu, Memory&);

#pragma region non-prefixed instructions
INSTR nop(Context& cpu, Memory& mem) {
	// TODO: m cycle?
	PRINTFUNC();
}

#pragma region 8-bit loads
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

	R8Reg dest = R8_FromBits(cpu.reg, mem, destVal);
	R8Reg src = R8_FromBits(cpu.reg, mem, srcVal);

	dest = src;

	cpu.MCycle();
}

INSTR ld_r8_imm8(Context& cpu, Memory& mem) {
	PRINTFUNC();

	byte destVal = (cpu.ir & 0b00'111'000) >> 3;
	R8Reg reg = R8_FromBits(cpu.reg, mem, destVal);

	reg = Read(cpu, mem);
	cpu.MCycle();
}

INSTR ld_acc_r16mem(Context& cpu, Memory& mem) {
	PRINTFUNC();

	byte destVal = (cpu.ir & 0b00'11'0000) >> 4;
	Addr16MemGetter handle = R16Mem_GetFromBits(destVal);

	byte data = mem[(cpu.reg.*handle)()];
	cpu.MCycle();

	cpu.reg.a = data;
	cpu.MCycle();
}

INSTR ld_r16mem_acc(Context& cpu, Memory& mem) {
	PRINTFUNC();

	byte destVal = (cpu.ir & 0b00'11'0000) >> 4;
	Addr16MemGetter handle = R16Mem_GetFromBits(destVal);

	mem[(cpu.reg.*handle)()] = cpu.reg.a;
}

INSTR ld_acc_imm16(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR ld_imm16_acc(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR ldh_acc_ffc(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR ldh_ffc_acc(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR ldh_acc_ffimm8(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR ldh_ffimm8_acc(Context& cpu, Memory& mem) {
	NOIMPL();
}
#pragma endregion 8-bit loads

#pragma region 16-bit loads
INSTR ld_r16_imm16(Context& cpu, Memory& mem) {
	PRINTFUNC();

	u16 data = Read2(cpu, mem);

	byte destVal = (cpu.ir & 0b00'11'0000) >> 4;
	Addr16Setter handle = R16_SetFromBits(destVal);
	
	(cpu.reg.*handle)(data);
	cpu.MCycle();
}

INSTR ld_imm16_sp(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR ld_sp_hl(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR ld_hl_spimm8(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR push_r16stk(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR pop_r16stk(Context& cpu, Memory& mem) {
	NOIMPL();
}
#pragma endregion 16-bit loads

#pragma region 8-bit arithmetic/logical instrucitons
INSTR add_r8(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR add_imm8(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR adc_r8(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR adc_imm8(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR sub_r8(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR sub_imm8(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR sbc_r8(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR sbc_imm8(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR cp_r8(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR cp_imm8(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR inc_r8(Context& cpu, Memory& mem) {
	PRINTFUNC();

	byte destVal = (cpu.ir & 0b00'111'000) >> 3;
	R8Reg reg = R8_FromBits(cpu.reg, mem, destVal);

	++reg;

	auto& flags = cpu.reg.f;
	flags.Zero = (reg == 0) ? 1 : 0;
	flags.Subtract = 0;
	flags.HalfCarry = (reg & 0b00001000) ? 1 : 0;

	cpu.MCycle();
}

INSTR dec_r8(Context& cpu, Memory& mem) {
	PRINTFUNC();

	byte destVal = (cpu.ir & 0b00'111'000) >> 3;
	R8Reg reg = R8_FromBits(cpu.reg, mem, destVal);

	--reg;

	auto& flags = cpu.reg.f;
	flags.Zero = (reg == 0) ? 1 : 0;
	flags.Subtract = 1;
	flags.HalfCarry = (reg & 0b00001000) ? 1 : 0;

	cpu.MCycle();
}

INSTR and_r8(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR and_imm8(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR or_r8(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR or_imm8(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR xor_r8(Context& cpu, Memory& mem) {
	PRINTFUNC();

	byte destVal = cpu.ir & 0b00000'111;
	R8Reg reg = R8_FromBits(cpu.reg, mem, destVal);

	cpu.reg.a = cpu.reg.a ^ reg;

	if (cpu.reg.a == 0)
		cpu.reg.f = 1 << 7; // Z000
	else
		cpu.reg.f = 0;		// 0000
}

INSTR xor_imm8(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR ccf(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR scf(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR daa(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR cpl(Context& cpu, Memory& mem) {
	NOIMPL();
}
#pragma endregion 8-bit arithmetic/logical instrucitons

#pragma region 16-bit arithmetic
INSTR inc_r16(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR dec_r16(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR add_hl_r16(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR add_sp_imm8(Context& cpu, Memory& mem) {
	NOIMPL();
}
#pragma endregion 16-bit arithmetic

#pragma region rotate, shift, bit manipulation
INSTR rlca(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR rrca(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR rla(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR rra(Context& cpu, Memory& mem) {
	NOIMPL();
}
#pragma endregion rotate, shift, bit manipulation

#pragma region control flow instructions
INSTR jp_imm16(Context& cpu, Memory& mem) {
	PRINTFUNC();

	u16 addr = Read2(cpu, mem);

	cpu.reg.pc = addr;
	cpu.MCycle();
}

INSTR jp_hl(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR jp_cond_imm16(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR jr_imm8(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR jr_cond_imm8(Context& cpu, Memory& mem) {
	PRINTFUNC();

	sbyte relativeAddr = Read(cpu, mem);
	byte condVal = (cpu.ir & 0b000'11'000) >> 3;

	if (FlagCond(cpu.reg.f, condVal)) {
		cpu.reg.pc += relativeAddr;
		cpu.MCycle();
	}

	cpu.MCycle();
}

INSTR call_imm16(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR call_cond_imm16(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR ret(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR ret_cond(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR reti(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR rst_tgt3(Context& cpu, Memory& mem) {
	NOIMPL();
}
#pragma endregion control flow instructions

#pragma region interrupt / halt related
INSTR stop(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR halt(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR di(Context& cpu, Memory& mem) {
	NOIMPL();
}

INSTR ei(Context& cpu, Memory& mem) {
	NOIMPL();
}
#pragma endregion interrupt / halt related

#pragma endregion non-prefixed instructions

#pragma region prefixed (cb) instructions
INSTR cb_rlc_r8(Context& cpu, Memory&) {
	NOIMPL();
}

INSTR cb_rrc_r8(Context& cpu, Memory&) {
	NOIMPL();
}

INSTR cb_rl_r8(Context& cpu, Memory&) {
	NOIMPL();
}

INSTR cb_rr_r8(Context& cpu, Memory&) {
	NOIMPL();
}

INSTR cb_sla_r8(Context& cpu, Memory&) {
	NOIMPL();
}

INSTR cb_sra_r8(Context& cpu, Memory&) {
	NOIMPL();
}

INSTR cb_swap_r8(Context& cpu, Memory&) {
	NOIMPL();
}

INSTR cb_srl_r8(Context& cpu, Memory&) {
	NOIMPL();
}

INSTR cb_bit_b3_r8(Context& cpu, Memory&) {
	NOIMPL();
}

INSTR cb_res_b3_r8(Context& cpu, Memory&) {
	NOIMPL();
}

INSTR cb_set_b3_r8(Context& cpu, Memory&) {
	NOIMPL();
}

INSTR cb_prefix(Context& cpu, Memory&) {
	NOIMPL();
}
#pragma endregion prefixed (cb) instructions

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
	INSTRMAP(ld_acc_imm16),
	INSTRMAP(ld_imm16_acc),
	INSTRMAP(ldh_acc_ffc),
	INSTRMAP(ldh_ffc_acc),
	INSTRMAP(ldh_acc_ffimm8),
	INSTRMAP(ldh_ffimm8_acc),
	INSTRMAP(ld_imm16_sp),
	INSTRMAP(ld_sp_hl),
	INSTRMAP(ld_hl_spimm8),
	INSTRMAP(add_imm8),
	INSTRMAP(adc_imm8),
	INSTRMAP(sub_imm8),
	INSTRMAP(sbc_imm8),
	INSTRMAP(cp_imm8),
	INSTRMAP(and_imm8),
	INSTRMAP(or_imm8),
	INSTRMAP(xor_imm8),
	INSTRMAP(ccf),
	INSTRMAP(scf),
	INSTRMAP(daa),
	INSTRMAP(cpl),
	INSTRMAP(add_sp_imm8),
	INSTRMAP(rlca),
	INSTRMAP(rrca),
	INSTRMAP(rrca),
	INSTRMAP(rla),
	INSTRMAP(rra),
	INSTRMAP(cb_prefix),
	INSTRMAP(jp_imm16),
	INSTRMAP(jp_hl),
	INSTRMAP(jr_imm8),
	INSTRMAP(call_imm16),
	INSTRMAP(ret),
	INSTRMAP(reti),
	INSTRMAP(stop),
	INSTRMAP(halt),
	INSTRMAP(di),
	INSTRMAP(ei),
});

// A mapping of all the instructions that have many possible op codes
// Stored as an array because find_if is used on this collection, which removes any benefit
// of using a map.
static constexpr auto variableInstrMap = std::to_array<VariableInstrData>(
{
	INSTRDATA(ld_r8_r8, 0b00'111'111),
	INSTRDATA(ld_r8_imm8, 0b00'111'000),
	INSTRDATA(ld_acc_r16mem, 0b00'11'0000),
	INSTRDATA(ld_r16mem_acc, 0b00'11'0000),
	INSTRDATA(ld_r16_imm16, 0b00'11'0000),
	INSTRDATA(push_r16stk, 0b00'11'0000),
	INSTRDATA(pop_r16stk, 0b00'11'0000),
	INSTRDATA(add_r8, 0b00000'111),
	INSTRDATA(adc_r8, 0b00000'111),
	INSTRDATA(sub_r8, 0b00000'111),
	INSTRDATA(sbc_r8, 0b00000'111),
	INSTRDATA(cp_r8, 0b00000'111),
	INSTRDATA(inc_r8, 0b00'111'000),
	INSTRDATA(dec_r8, 0b00'111'000),
	INSTRDATA(and_r8, 0b00000'111),
	INSTRDATA(or_r8, 0b00000'111),
	INSTRDATA(xor_r8, 0b00000'111),
	INSTRDATA(inc_r16, 0b00'11'0000),
	INSTRDATA(dec_r16, 0b00'11'0000),
	INSTRDATA(add_hl_r16, 0b00'11'0000),
	INSTRDATA(jp_cond_imm16, 0b000'11'000),
	INSTRDATA(jr_cond_imm8, 0b000'11'000),
	INSTRDATA(call_cond_imm16, 0b000'11'000),
	INSTRDATA(ret_cond, 0b000'11'000),
	INSTRDATA(rst_tgt3, 0b00'111'000),
});

// A mapping of all the cb instructions that have many possible op codes.
// Stored as an array for the same reason as variableInstrMap
static constexpr auto cbInstrMap = std::to_array<VariableInstrData>(
{
	INSTRDATA(cb_rlc_r8, 0b00000'111),
	INSTRDATA(cb_rrc_r8, 0b00000'111),
	INSTRDATA(cb_rl_r8, 0b00000'111),
	INSTRDATA(cb_rr_r8, 0b00000'111),
	INSTRDATA(cb_sla_r8, 0b00000'111),
	INSTRDATA(cb_sra_r8, 0b00000'111),
	INSTRDATA(cb_swap_r8, 0b00000'111),
	INSTRDATA(cb_srl_r8, 0b00000'111),
	INSTRDATA(cb_bit_b3_r8, 0b00'111'111),
	INSTRDATA(cb_res_b3_r8, 0b00'111'111),
	INSTRDATA(cb_set_b3_r8, 0b00'111'111),
});

#undef INSTRMAP

// A list of unused op codes. If an op code in this list is somehow chosen,
// the cpu should hang.
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
		debug::cexpr::println(stderr, "CPU should hang");

		_handler = nullptr;
		return false;
	}

	// First, check if it's a non-variable instruction
	if (auto it = constInstrMap.find(opCode); it != constInstrMap.end()) {
		debug::cexpr::println("Op Code (ir): {:#010b} ({:#04x})\tFound: {:#010b} ({:#04x})",
							  ir, ir, static_cast<byte>(it->first), static_cast<byte>(it->first));

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
		debug::cexpr::forceprinterr("Couldn't find instruction! ");
		debug::cexpr::forceprinterr("Op Code (ir): {:#010b} ({:#04x})\n", ir, ir);

		_handler = nullptr;
		return false;
	}

	debug::cexpr::println("Op Code (ir): {:#010b} ({:#04x})\tFound: {:#010b} ({:#04x})",
						  ir, ir, static_cast<byte>(it->op), static_cast<byte>(it->op));
	// Store a function pointer rather than updating the ir
	// because some instructions have variable op codes. The ir still holds the bits
	// to determine what register / data / condition needs to be used.
	_handler = it->handler;

	return true;
}

bool Context::Exec() {
	if (!_handler) {
		debug::cexpr::println("Invalid instruction!");
		return false;
	}

	_handler(*this, _memory);

	return true;
}

} // namespace gb::cpu
