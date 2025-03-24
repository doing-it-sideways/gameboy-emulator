#pragma once

#include "Core.hpp"

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
			- bit 6 (n): subtraction flag (bcd) - see gbdev
			- bit 5 (h): half carry flag (bcd) - see gbdev
			- bit 4 (c / cy): carry flag
	- general-purpose registers (16-bit, 8 bit halves)
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
	struct RegisterFile {
		u16 pc;
		u16 sp;
		u8 a, f;
		u8 b, c;
		u8 d, e;
		u8 h, l;
	} reg;

	Context();

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


};

enum class OpCode : byte {
	nop = 0x00,
};

} // namespace gb::cpu
