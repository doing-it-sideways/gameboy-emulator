#pragma once

#include "Core.hpp"
#include "BitfieldStruct.hpp"
#include "Timer.hpp"

namespace gb {

struct HWRegs {
// ----- Structs and Typedefs -----
	BITFIELD_UNION_BYTE(SerialControl, flags,
		byte ClockSelect : 1;
		byte ClockSpeed : 1;
		byte _ : 5; // unused
		byte TransferEnable : 1;
	);

	BITFIELD_UNION_BYTE(InterruptFlags, flags,
		byte VBlankInt : 1;
		byte LCDInt : 1;
		byte TimerInt : 1;
		byte SerialInt : 1;
		byte JoypadInt : 1;
		byte _ : 3; // unused
	);

// ----- Vars -----
	// In order of their memory locations:

	byte sb;			// $FF01 -- Serial Transfer Data

	SerialControl sc;	// $FF02

	Timer& timer;		// [$FF04, $FF07]

	InterruptFlags iF;	// $FF0F

	InterruptFlags ie;	// $FFFF

// ----- Funcs -----
	static HWRegs InitRegs(Timer& emuTimer, bool isCGB = false);

	void Reset(byte oldOAMReg, bool isCGB = false);

	byte& Read(u16 addr);
	void Write(u16 addr, byte val);
};

} // namespace gb

#undef BITFIELD_UNION_BYTE
