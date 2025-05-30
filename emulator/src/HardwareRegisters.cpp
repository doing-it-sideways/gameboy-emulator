#include "HardwareRegisters.hpp"
#include "ConstexprAdditions.hpp"

namespace gb {

HWRegs HWRegs::InitRegs(bool isCGB) {
	if (isCGB)
		return {}; // TODO
	
	return HWRegs{
		.sb = 0x00,
		.sc = 0x7E,
		.iF = 0xE1,
		.ie = 0x00
	};
}

void HWRegs::Reset([[maybe_unused]] byte oldOAMReg, bool isCGB) {
	*this = InitRegs(isCGB);
}

byte& HWRegs::Read(u16 addr) {
	if (addr >= 0xFF04 && addr <= 0xFF07)
		return timer.Read(addr);

	switch (addr) {
	case 0xFF01: return sb;
	case 0xFF02: return sc.asByte;
	case 0xFF0F: return iF.asByte;
	case 0xFFFF: return ie.asByte;
	default:
		debug::cexpr::println("Unimplemented or invalid io read at {:#06x}", addr);
		debug::cexpr::exit(EXIT_FAILURE);
		std::unreachable();
	}
}

void HWRegs::Write(u16 addr, byte val) {
	if (addr >= 0xFF04 && addr <= 0xFF07) {
		timer.Write(addr, val);
		return;
	}

	switch (addr) {
	case 0xFF01:
		sb = val;
		return;
	case 0xFF02:
		sc = val;
		return;
	case 0xFF0F:
		iF = val;
		return;
	case 0xFFFF:
		ie = val;
		return;
	default:
		debug::cexpr::println("Unimplemented or invalid io write at {:#06x}", addr);
		debug::cexpr::exit(EXIT_FAILURE);
		std::unreachable();
	}
}

} // namespace gb
