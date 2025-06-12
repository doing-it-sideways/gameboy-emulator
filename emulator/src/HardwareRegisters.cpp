#include "HardwareRegisters.hpp"
#include "ConstexprAdditions.hpp"

namespace gb {

HWRegs HWRegs::InitRegs(Timer& emuTimer, [[maybe_unused]] bool isCGB) {
	//if (isCGB)
	//	return {}; // TODO
	
	return HWRegs {
		.sb = 0x00,
		.sc = 0x7E,
		.timer = emuTimer,
		.iF = 0xE1,
		.ie = 0x00
	};
}

void HWRegs::Reset([[maybe_unused]] byte oldOAMReg, [[maybe_unused]] bool isCGB) {
	// TODO
	debug::cexpr::println(stderr, "hardware registers not reset");
}

byte& HWRegs::Read(u16 addr) {
	static byte unimplByte = 0;
	static byte gameboyDoctorLCDTemp = 0x90;
	if (addr >= 0xFF04 && addr <= 0xFF07)
		return timer.Read(addr);

	switch (addr) {
	case 0xFF01: return sb;
	case 0xFF02: return sc.asByte;
	case 0xFF0F: return iF.asByte;
	case 0xFF44: return gameboyDoctorLCDTemp; // gameboy doctor tests TODO: remove
	case 0xFFFF: return ie.asByte;
	default:
		debug::cexpr::println(stderr, "Unimplemented or invalid io read at {:#06x}", addr);
		//debug::cexpr::exit(EXIT_FAILURE);
		//std::unreachable();
		return unimplByte;
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
		debug::cexpr::println(stderr, "Unimplemented or invalid io write at {:#06x}", addr);
		//debug::cexpr::exit(EXIT_FAILURE);
		//std::unreachable();
		return;
	}
}

} // namespace gb
