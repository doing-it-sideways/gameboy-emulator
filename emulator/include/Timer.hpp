#pragma once

#include "Core.hpp"

namespace gb {

struct Timer {
// ----- Structs and Typedefs -----
	struct TimerControl {
		union {
			struct {
				byte _ : 5; // unused
				byte Enable : 1;
				byte ClockSelect : 2;
			} data;
			byte asByte;
		};

		constexpr operator byte() const { return asByte; }

		constexpr TimerControl() {};
		constexpr TimerControl(byte b) : asByte(b) {}
		constexpr TimerControl& operator=(byte b) {
			asByte = b;
			return *this;
		}
	};

// ----- Vars -----
	byte div;	// Divider register
	byte tima;	// Timer counter
	byte tma;	// Timer modulo

	TimerControl tac;

// ----- Funcs -----
	Timer(bool isCGB = false);

	// true == interrupt request needed, false == no interrupt request needed
	bool Tick();

	byte& Read(u16 addr);
	void Write(u16 addr, byte data);
};

} // namespace gb
