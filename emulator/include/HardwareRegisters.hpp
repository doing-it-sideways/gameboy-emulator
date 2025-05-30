#pragma once

#include "Core.hpp"
#include "Timer.hpp"

#define BITFIELD_UNION_BYTE(StructName, BitfieldName, Bitfield) \
	struct StructName { \
		union { \
			struct { \
				Bitfield \
			} BitfieldName; \
			byte asByte; \
		}; \
		constexpr operator byte() const { return asByte; } \
		constexpr StructName##& operator=(byte b) { asByte = b; return *this; } \
	}
	

namespace gb {

struct HWRegs {
// ----- Structs and Typedefs -----
	BITFIELD_UNION_BYTE(SerialControl, flags,
		byte TransferEnable : 1;
		byte Unused : 5; // unused
		byte ClockSpeed : 1;
		byte ClockSelect : 1;
	);

	BITFIELD_UNION_BYTE(InterruptFlags, flags,
		byte Unused : 3; // unused
		byte JoypadInt : 1;
		byte SerialInt : 1;
		byte TimerInt : 1;
		byte LCDInt : 1;
		byte VBlankInt : 1;
	);
// ----- Vars -----
	// In order of their memory locations:

	byte sb;			// $FF01 -- Serial Transfer Data

	SerialControl sc;	// $FF02

	Timer timer;		// [$FF04, $FF07]

	InterruptFlags iF;	// $FF0F

	InterruptFlags ie;	// $FFFF

// ----- Funcs -----
	static HWRegs InitRegs(bool isCGB = false);
	void Reset(byte oldOAMReg, bool isCGB = false);

	byte& Read(u16 addr);
	void Write(u16 addr, byte val);
};

} // namespace gb

#undef BITFIELD_UNION_BYTE
