#pragma once

#include "Core.hpp"
#include <vector>

namespace gb::cpu {
/* 
control unit(decodes instructions)
register file (holds cpu state in registers)
	- 16-bit program counter (pc)
	- 16-bit stack pointer (sp)
	- 16-bit af register (af)
		- 8-bit accumulator (a)
		- 8-bit flag register (f)
			- bit 7 (z): zero flag
			- bit 6 (n): subtraction flag (bcd) - see gbdev.io
			- bit 5 (h): half carry flag (bcd) - see gbdev.io
			- bit 4 (c / cy): carry flag
	- general-purpose registers (16-bit, 8-bit halves)
		- bc
		- de
		- hl
	- 8-bit instruction register (ir)
	- 8 bit interrupt enable (ie)
8-bit arithmetic logic unit (alu)
	- two 8-bit input ports
	- result -> register file or cpu data bus
16-bit increment/decrement unit
	- only performs ++/-- ops on 16-bit address bus value
	- independent of alu
	- result -> register file. writes to register pair/16-bit register
*/

/*//works on little-endian systems but technically is ub
* struct ConvRegister {
*	union {
*		u16 reg;
*		struct {
*			u8 a;
*			u8 b;
*		} as8;
*	};
* };
* // (static_cast<u16>(a) << 8) + b == reg
*/

struct Context {
// --- Structs and Typedefs ---
	struct RegisterFile {
		u16 pc;
		u16 sp;
		byte a, f; // f could also be a bit-field, but not very wieldy
		byte b, c;
		byte d, e;
		byte h, l;
	};

	enum class Flags : byte {
		Zero = 7,
		Sub = 6,
		HalfCarry = 5,
		Carry = 4
	};

// --- Vars ---
	RegisterFile reg;

	// Instruction register, interrupt enable
	byte ir, ie;

#pragma region 16 bit register definitions
#define REGISTER16(r1, r2) \
	inline u16 r1##r2() const { return (static_cast<u16>(reg.##r1) << 8) + reg.##r2; } \
	inline void r1##r2(u16 val) { \
		reg.##r1 = static_cast<u8>((val & 0xFF00) >> 8); \
		reg.##r2 = static_cast<u8>(val & 0x00FF); \
	}

	//REGISTER16(a, f); // low byte undefined for af
	REGISTER16(b, c);
	REGISTER16(d, e);
	REGISTER16(h, l);

#undef REGISTER16
#pragma endregion

// --- Functions ---
	// Mimic booting up the cpu
	constexpr Context();

	// Increment and Decrement functions for 16-bit addresses
	constexpr void Inc(u16& reg);
	constexpr void Dec(u16& reg);

	// Shorthand for setting flag register
	constexpr void SetFlag(Flags flag, bool set);
};

// https://gbdev.io/pandocs/CPU_Instruction_Set.html
// grouped by classification from gekkio.fi complete technical reference
enum class OpCode : byte {
	nop,

	// 8-bit loads
	ld_r8_r8,		// data from reg to reg
	ld_r8_imm8,		// data to reg
	ld_acc_r16mem,	// data from abs address (register) to accumulator
	ld_r16mem_acc,	// data from accumulator to abs address (register)
	ld_acc_imm16,	// data from abs address (constant) to accumulator
	ld_imm16_a,		// data from accumulator to abs address (constant)
	ldh_acc_ffc,	// data from ($FF00 + c) to accumulator
	ldh_ffc_acc,	// data from accumulator to ($FF00 + c)
	ldh_acc_ffimm8,	// data from ($FF00 + n) to accumulator
	ldh_ffimm8_acc,	// data from accumulator to ($FF00 + n)
	ld_acc_hld,		// data from hl-- to accumulator
	ld_hld_acc,		// data from accumulator to hl--
	ld_acc_hli,		// data from hl++ to accumulator
	ld_hli_acc,		// data from accumulator to hl++

	// 16-bit loads
	ld_r16_imm16,	// data to reg
	ld_imm16_sp,	// data from sp to abs address (constant)
	ld_sp_hl,		// data from hl to stack pointer
	ld_hl_spimm8,	// data from (sp + n) to hl
	push_r16stk,	// push to stack
	pop_r16stk,		// pop from stack

	// 8-bit arithmetic/logical
	add_r8,			// acc += register
	add_imm8,		// acc += n
	adc_r8,			// acc += register + carry
	adc_imm8,		// acc += n + carry
	sub_r8,			// acc -= register
	sub_imm8,		// acc -= n
	sbc_r8,			// acc = acc - register - carry
	sbc_imm8,		// acc = acc - n - carry
	cp_r8,			// set flags for comparison between acc and register
	cp_imm8,		// set flags for comparison between acc and n
	inc_r8,			// ++register
	dec_r8,			// --register
	and_r8,			// acc &= reg
	and_imm8,		// acc &= n
	or_r8,			// acc |= reg
	or_imm8,		// acc |= n
	xor_r8,			// acc ^= reg
	xor_imm8,		// acc ^= n
	ccf,			// complement carry flag
	scf,			// set carry flag
	daa,			// decimal adjust accumulator
	cpl,			// complement accumulator

	// 16-bit arithmetic
	inc_r16,		// ++register
	dec_r16,		// --register
	add_hl_r16,		// hl += n
	add_sp_imm8,	// sp += e

	// rotate, shift, bit


	// control flow

	// interrupt / halt related
	stop,
	halt,
	di, // disable interrupts
	ei, // enable interrupts

	undefined // $D3, $DB, $DD, $E3, $E4, $EB, $EC, $ED, $F4, $FC, and $FD
};

} // namespace gb::cpu
