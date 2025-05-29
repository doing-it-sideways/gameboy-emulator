#include "Timer.hpp"
#include "ConstexprAdditions.hpp"

namespace gb {

Timer::Timer(bool isCGB)
	: div(isCGB ? 0x00 : 0xAB) // CGB ?
	, tima(0x00)
	, tma(0x00)
	, tac(0xF8)
{}

bool Timer::Tick() {
	return false; // TODO
}

byte& Timer::Read(u16 addr) {
	switch (addr) {
	case 0xFF04: return div;
	case 0xFF05: return tima;
	case 0xFF06: return tma;
	case 0xFF07: return tac.asByte;
	default:
		debug::cexpr::println("Unimplemented or invalid memory access at {:#06x}", addr);
		debug::cexpr::exit(EXIT_FAILURE);
		std::unreachable();
	}
}

void Timer::Write(u16 addr, byte data) {
	switch (addr) {
	case 0xFF04:
		div = 0; // any write resets div
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
		debug::cexpr::println("Unimplemented or invalid memory access at {:#06x}", addr);
		debug::cexpr::exit(EXIT_FAILURE);
		std::unreachable();
	}
}

} // namespace gb
