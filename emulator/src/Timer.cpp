#include "Timer.hpp"
#include "ConstexprAdditions.hpp"

namespace gb {

Timer::Timer(bool isCGB)
	: divWhole(isCGB ? 0x0000 : 0xAB00) // CGB ?
	, tima(0x00)
	, tma(0x00)
	, tac(0xF8)
{}

// TODO: Timer obscure behavior:
// https://gbdev.io/pandocs/Timer_Obscure_Behaviour.html
bool Timer::Tick() {
	u16 prev = divWhole++;

	if (tac.data.Enable == 0)
		return false;

	// 4 tcycles per mcycle
	static constexpr u16 incRate[4] = { 256 << 1, 4 << 1, 16 << 1, 64 << 1 };

	if ((prev & incRate[tac.data.ClockSelect]) != 0 && (divWhole & incRate[tac.data.ClockSelect]) == 0)
		return Update();

	return false;
}

bool Timer::Update() {
	// TODO: If a TMA write is executed on the same M-cycle as the content of TMA
	// is transferred to TIMA due to a timer overflow, the old value
	// is transferred to TIMA.
	if (byte oldTima = tima++; tima == 0xFF) {
		tima = tma;
		return true;
	}

	return false;
}

byte& Timer::Read(u16 addr) {
	switch (addr) {
	case 0xFF04: return div.upper;
	case 0xFF05: return tima;
	case 0xFF06: return tma;
	case 0xFF07: return tac.asByte;
	default:
		debug::cexpr::println("Unimplemented or invalid timer read at {:#06x}", addr);
		debug::cexpr::exit(EXIT_FAILURE);
		std::unreachable();
	}
}

void Timer::Write(u16 addr, byte data) {
	switch (addr) {
	case 0xFF04:
		// any write resets div
		divWhole = 0;
		return;
	case 0xFF05:
		tima = data;
		return;
	case 0xFF06:
		tma = data;
		return;
	case 0xFF07:
		tac = data;
		return;
	default:
		debug::cexpr::println("Unimplemented or invalid timer write at {:#06x}", addr);
		debug::cexpr::exit(EXIT_FAILURE);
		std::unreachable();
	}
}

} // namespace gb
