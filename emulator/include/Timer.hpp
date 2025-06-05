#pragma once

#include "Core.hpp"
#include "BitfieldStruct.hpp"

namespace gb {

struct Timer {
// ----- Structs and Typedefs -----
	BITFIELD_UNION_BYTE(TimerControl, data,
		byte ClockSelect : 2;
		byte Enable : 1;
		byte _ : 5;
	);

// ----- Vars -----
	// upper: basic clock frequency, lower : thing that increments
	// Divider register
	union {
		struct {
			byte upper, lower;
		} div;
		u16 divWhole;
	};

	byte tima;	// Timer counter
	byte tma;	// Timer modulo

	TimerControl tac;

// ----- Funcs -----
	Timer(bool isCGB = false);

	// true == interrupt request needed, false == no interrupt request needed
	bool Tick();

	byte& Read(u16 addr);
	void Write(u16 addr, byte data);

private:
	bool Update();
};

} // namespace gb
