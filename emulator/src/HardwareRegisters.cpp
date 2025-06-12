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
		.lcdc = 0x91,
		.stat = 0x85,
		.scy = 0x00,
		.scx = 0x00,
		.ly = 0x00,
		.lyc = 0x00,
		.wy = 0x00,
		.wx = 0x00,
		.ie = 0x00
	};
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
	case 0xFF40: return lcdc.asByte;
	case 0xFF41: return stat.asByte;
	case 0xFF42: return scy;
	case 0xFF43: return scx;
	case 0xFF44: return gameboyDoctorLCDTemp; // gameboy doctor tests TODO: remove
	case 0xFF45: return lyc;
	case 0xFF4A: return wy;
	case 0xFF4B: return wx;
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
