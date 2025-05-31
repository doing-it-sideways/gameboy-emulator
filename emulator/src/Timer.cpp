#include "Timer.hpp"
#include "ConstexprAdditions.hpp"

namespace gb {

Timer::Timer(bool isCGB)
	: divUpper(isCGB ? 0x00 : 0xAB) // CGB ?
	, divLower(0x00)
	, tima(0x00)
	, tma(0x00)
	, tac(0xF8)
{}

// TODO: Timer obscure behavior:
// https://gbdev.io/pandocs/Timer_Obscure_Behaviour.html
bool Timer::Tick() {
	return false;
}

byte& Timer::Read(u16 addr) {
	switch (addr) {
	case 0xFF04: return divUpper;
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
		divUpper = 0;
		divLower = 0;
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
