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
		.dma = 0xFF,
		.bgp = 0xFC,
		.obp0 = 0xFF,
		.obp1 = 0xFF,
		.wy = 0x00,
		.wx = 0x00,
		.ie = 0x00
	};
}

byte& HWRegs::Read(u16 addr) {
	static byte invalidByte = 0xFF;
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
	case 0xFF44: return ly;
	case 0xFF45: return lyc;
	case 0xFF46: return dma;
	case 0xFF47: return bgp.asByte;
	case 0xFF48: return obp0.asByte;
	case 0xFF49: return obp1.asByte;
	case 0xFF4A: return wy;
	case 0xFF4B: return wx;
	case 0xFFFF: return ie.asByte;
	default:
		debug::cexpr::println(stderr, "Unimplemented or invalid io read at {:#06x}", addr);
		//debug::cexpr::exit(EXIT_FAILURE);
		//std::unreachable();
		return invalidByte;
	}
}

void HWRegs::Write(u16 addr, byte val) {
	if (addr >= 0xFF04 && addr <= 0xFF07) {
		timer.Write(addr, val);
		return;
	}

	switch (addr) {
	case 0xFF01: sb = val; return;
	case 0xFF02: sc = val; return;
	case 0xFF0F: iF = val; return;
	case 0xFF40: lcdc = val; return;
	case 0xFF41: {
		// TODO: spurious STAT interrupts
		// https://gbdev.io/pandocs/STAT.html#spurious-stat-interrupts

		// Undocumented bug, needed for some games
		if (stat.flags.PPUMode < 2 && lcdc.flags.LCDEnable == 1)
			iF.flags.LCDInt = 1;

		LCDStatus newStat = static_cast<LCDStatus>(val);
		stat.flags.LycIntSelect = newStat.flags.LycIntSelect;
		stat.flags.M0Select = newStat.flags.M0Select;
		stat.flags.M1Select = newStat.flags.M1Select;
		stat.flags.M2Select = newStat.flags.M2Select;
		return;
	}
	case 0xFF42: scy = val; return;
	case 0xFF43: scx = val; return;
	case 0xFF44: ly = 0; return; // UNDEFINED: resets scanline
	case 0xFF45: lyc = val; return;
	case 0xFF46: dma = val; return;
	case 0xFF47: bgp = val; return;
	case 0xFF48: obp0 = val & (~3); return; // lower two bits are ignored -> transparent
	case 0xFF49: obp1 = val & (~3); return; // lower two bits are ignored -> transparent
	case 0xFF4A: wy = val; return;
	case 0xFF4B: wx = val; return;
	case 0xFFFF: ie = val; return;
	default:
		debug::cexpr::println(stderr, "Unimplemented or invalid io write at {:#06x}", addr);
		//debug::cexpr::exit(EXIT_FAILURE);
		//std::unreachable();
		return;
	}
}

} // namespace gb
