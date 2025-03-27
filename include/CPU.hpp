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
	struct Flags {
		// bytes instead of bools so they can all be set with a single number
		byte Zero : 1;
		byte Subtract : 1;
		byte HalfCarry : 1;
		byte Carry : 1;
		byte : 4; // unused
	};

	struct RegisterFile {
		u16 pc;
		u16 sp;
		byte a;//, f; // f could also be a bit-field, but not very wieldy
		Flags f;

		byte b, c;
		byte d, e;
		byte h, l;
	};

// --- Vars ---
	RegisterFile reg;

	// Instruction register, interrupt enable
	byte ir, ie;

#pragma region 16 bit register definitions
#define REGISTER16(r1, r2) \
	constexpr inline u16 r1##r2() const { return (static_cast<u16>(reg.##r1) << 8) + reg.##r2; } \
	constexpr inline void r1##r2(u16 val) { \
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
	constexpr void Inc(u16 reg);
	constexpr void Dec(u16 reg);

	constexpr void Exec();
};

/*
	https://gbdev.io/pandocs/CPU_Instruction_Set.html
	grouped by classification from gekkio.fi complete technical reference
	instructions that have variable bits will wrap the bits with '
		ex: 0b11'000'110 <-- middle 3 0s can be either 0 or 1 depending on the exact instruction
		the exceptions are ld_r8_r8, some 8-bit arithmetic instructions, and cb prefix instructions.
		the last ' has to be dropped due to language constraints
*/
enum class OpCode : byte {
	// https://gist.github.com/SonoSooS/c0055300670d678b5ae8433e20bea595#nop-and-stop
	nop =				0b00000000,

	// 8-bit loads
	ld_r8_r8 =			0b01'000'000,	// data from reg to reg.
										//note: ld [hl], [hl] will yield the halt instruction
	ld_r8_imm8 =		0b00'000'110,	// data to reg
	ld_acc_r16mem =		0b00'00'1010,	// data from abs address (register) to accumulator
	ld_r16mem_acc =		0b00'00'0010,	// data from accumulator to abs address (register)
	ld_acc_imm16 =		0b11111010,		// data from abs address (constant) to accumulator
	ld_imm16_acc =		0b11101010,		// data from accumulator to abs address (constant)
	ldh_acc_ffc =		0b11110010,		// data from ($FF00 + c) to accumulator
	ldh_ffc_acc =		0b11100010,		// data from accumulator to ($FF00 + c)
	ldh_acc_ffimm8 =	0b11110000,		// data from ($FF00 + n) to accumulator
	ldh_ffimm8_acc =	0b11100000,		// data from accumulator to ($FF00 + n)

	// 16-bit loads
	ld_r16_imm16 =		0b00'00'0001,	// data to reg
	ld_imm16_sp =		0b00001000,		// data from sp to abs address (constant)
	ld_sp_hl =			0b11111001,		// data from hl to stack pointer
	ld_hl_spimm8 =		0b11111000,		// data from (sp + n) to hl
	push_r16stk =		0b11'00'0101,	// push to stack
	pop_r16stk =		0b11'00'0001,	// pop from stack

	// 8-bit arithmetic/logical
	add_r8 =			0b100000'00,	// acc += register
	add_imm8 =			0b11000110,		// acc += n
	adc_r8 =			0b100010'00,	// acc += register + carry
	adc_imm8 =			0b11001110,		// acc += n + carry
	sub_r8 =			0b100100'00,	// acc -= register
	sub_imm8 =			0b11010110,		// acc -= n
	sbc_r8 =			0b100110'00,	// acc = acc - register - carry
	sbc_imm8 =			0b11011110,		// acc = acc - n - carry
	cp_r8 =				0b101110'00,	// set flags for comparison between acc and register
	cp_imm8 =			0b11111110,		// set flags for comparison between acc and n
	inc_r8 =			0b00'00'0100,	// ++register
	dec_r8 =			0b00'00'0101,	// --register
	and_r8 =			0b101000'00,	// acc &= reg
	and_imm8 =			0b11100110,		// acc &= n
	or_r8 =				0b101100'00,	// acc |= reg
	or_imm8 =			0b11110110,		// acc |= n
	xor_r8 =			0b101010'00,	// acc ^= reg
	xor_imm8 =			0b11101110,		// acc ^= n
	ccf =				0b00111111,		// complement carry flag
	scf =				0b00110111,		// set carry flag
	daa =				0b00100111,		// decimal adjust accumulator
	cpl =				0b00101111,		// complement accumulator

	// 16-bit arithmetic
	inc_r16 =			0b00'00'0011,	// ++register
	dec_r16 =			0b00'00'1011,	// --register
	add_hl_r16 =		0b00'00'1001,	// hl += n
	add_sp_imm8 =		0b11101000,		// sp += e

	// rotate, shift, bit manipulation
	rlca =				0b00000111,		// rotate acc left circular
	rrca =				0b00001111,		// rotate acc right circular
	rla =				0b00010111,		// rotate acc left (carry = lost data)
	rra =				0b00011111,		// rotate acc right (carry = lost data)
	// instructions prefixed cb have their instructions stored in a separate table
	cb_prefix =			0b11001011,		// tells cpu that instruction is in a separate table
	cb_rlc_r8 =			0b00000'000,	// rotate reg left circular
	cb_rrc_r8 =			0b00001'000,	// rotate reg right circular
	cb_rl_r8 =			0b00010'000,	// rotate reg left (carry = lost data)
	cb_rr_r8 =			0b00011'000,	// rotate reg right (carry = lost data)
	cb_sla_r8 =			0b00100'000,	// reg <<= 1
	cb_sra_r8 =			0b00101'000,	// reg >>= 1 (abcd -> xabc. x = a)
	cb_swap_r8 =		0b00110'000,	// swap nibbles in reg
	cb_srl_r8 =			0b00111'000,	// reg >>= 1 (abcd -> xabc. x = 0)
	cb_bit_b3_r8 =		0b01'000'000,	// test bit b3 in reg == 1
	cb_res_b3_r8 =		0b10'000'000,	// reset bit b3 in reg
	cb_set_b3_r8 =		0b11'000'000,	// set bit b3 in reg

	// control flow
	jp_imm16 =			0b11000011,		// jump to abs address n
	jp_hl =				0b11101001,		// jump to address stored in hl
	jp_cond_imm16 =		0b110'00'010,	// conditional jump to abs address n depending on condition
	jr_imm8 =			0b00011000,		// jump to pc + n (n is signed)
	jr_cond_imm8 =		0b001'00'000,	// conditional jump to abs address n (n is signed) depending condition
	call_imm16 =		0b11001101,		// function call to abs address n
	call_cond_imm16 =	0b110'00'100,	// conditional call to abs address n depending on condition
	ret =				0b11001001,		// return from a function
	ret_cond =			0b110'00'000,	// return from a function depending on condition
	reti =				0b11011001,		// return from a function and enable interrupts (ei + ret)
	rst_tgt3 =			0b11'000'111,	// function call to abs fixed address tgt3 (see gbdev.io)

	// interrupt / halt related
	stop =				0b00010000,		// enter cpu very low-power mode. behavior is fragile.
					// https://gbdev.io/pandocs/Reducing_Power_Consumption.html#using-the-stop-instruction
					// or https://gist.github.com/SonoSooS/c0055300670d678b5ae8433e20bea595#nop-and-stop
	halt =				0b01110110,		// enter cpu low-power mode until interrupt occurs.
										//see: https://rgbds.gbdev.io/docs/v0.9.1/gbz80.7#HALT
										// note: ld [hl], [hl] will yield the halt instruction
	di =				0b11110011,		// disable interrupts
	ei =				0b11111011,		// enable interrupts

	undefined // $D3, $DB, $DD, $E3, $E4, $EB, $EC, $ED, $F4, $FC, and $FD
};

} // namespace gb::cpu
