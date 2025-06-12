#pragma once

#include "Core.hpp"
#include "BitfieldStruct.hpp"
#include "Timer.hpp"

namespace gb {

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

BITFIELD_UNION_BYTE(LCDControl, flags,
	byte BGWindowEnable : 1;
	byte OBJEnable : 1;
	byte OBJSize : 1;
	byte BGTileMapArea : 1;
	byte BGWindowTilemapArea : 1;
	byte WindowEnable : 1;
	byte WindowTilemapArea : 1;
	byte LCDEnable : 1;
);

BITFIELD_UNION_BYTE(LCDStatus, flags,
	byte PPUMode : 2; // read only
	byte LycEqLy : 1; // read only
	byte M0Select : 1;
	byte M1Select : 1;
	byte M2Select : 1;
	byte LycIntSelect : 1;
	byte _ : 1; // unused
);

struct HWRegs {
	// In order of their memory locations:
	byte sb;			// $FF01 -- Serial Transfer Data
	SerialControl sc;	// $FF02

	Timer& timer;		// [$FF04, $FF07]

	InterruptFlags iF;	// $FF0F

	LCDControl lcdc;	// $FF40
	LCDStatus stat;		// $FF41
	byte scy;			// $FF42 -- Background viewport y pos
	byte scx;			// $FF43 -- Background viewport x pos
	byte ly;			// $FF44 -- LCD Y coordinate (read-only)
	byte lyc;			// $FF45 -- LY compare
	byte wy;			// $FF4A -- Window y pos
	byte wx;			// $FF4B -- Window x pos + 7

	InterruptFlags ie;	// $FFFF

// ----- Funcs -----
	static HWRegs InitRegs(Timer& emuTimer, bool isCGB = false);

	byte& Read(u16 addr);
	void Write(u16 addr, byte val);
};

} // namespace gb
