#include <utility>

#include "Memory.hpp"
#include "ConstexprAdditions.hpp"

namespace gb {

static constexpr u16 interruptEnd = 0x0100;	// restarts and interrupts [$0000, $0FFF]
static constexpr u16 headerEnd = 0x0150;	// cartridge header [$0100, $014F]
static constexpr u16 rom0End = 0x4000;		// fixed 16 KiB ROM bank [$0150, $3FFF]
static constexpr u16 romNEnd = 0x8000;		// switchable 16 KiB ROM bank [$4000, $7FFF]
static constexpr u16 vramEnd = 0xA000;		// 8 KiB VRAM [$8000, $9FFF]
static constexpr u16 ramExtEnd = 0xC000;	// 8 KiB Cartridge ram [$A000, $BFFF]
static constexpr u16 ramWork0End = 0xD000;	// [$C000, $CFFF]
static constexpr u16 ramWork1End = 0xE000;	// [$D000, $DFFF]
static constexpr u16 unusable0End = 0xFE00;	// Prohibited to use. [$E000, $FDFF]
static constexpr u16 oamEnd = 0xFEA0;		// object attribute memory [$FE00, $FE9F]
static constexpr u16 unusable1End = 0xFF00;	// Prohibited to use. [$FEA0, $FEFF]
static constexpr u16 ioEnd = 0xFF80;		// io registers [$FF00, $FF7F]
static constexpr u16 hramEnd = 0xFFFF;		// High RAM (Zero page) [$FF80, $FFFE]
static constexpr u16 regIE = 0xFFFF;		// Interrupt enable register ($FFFF)

static constexpr u16 romBankSize = 0x7FFF - 0x4000 + 1;
static constexpr u16 cartRamBankSize = 0xBFFF - 0xA000 + 1;
static constexpr u16 sysRamBankSize = 0xCFFF - 0xC000 + 1;

/*	
	The register names are taken from gekkio.fi's gameboy complete
	technical reference. Writing values to these registers doesn't actually write
	to memory; they instead change behavior in the chip. Details of what they do are
	in the document previously mentioned.
*/

namespace mbc1 {
namespace reg {

static struct BankNum {
	u8 : 1;
	u8 bank2 : 2 = 0;
	u8 bank1 : 5 = 1;

	// 0xx yyyyy
	//  ^ bank2
	//     ^ bank1
	byte operator()() const { return bank2 << 5 | bank1; }
} bankNum;

static bool ramEnabled = false;
static byte mode = false;

} // namespace reg

static constexpr u16 ramEnableEnd = 0x2000;
static constexpr u16 romBank1End = 0x4000;
static constexpr u16 romBank2End = 0x6000;
static constexpr u16 ramBankEnd = romBank2End;
static constexpr u16 bankingModeEnd = 0x8000;

} // namespace mbc1

static MapperChip GetMapperChip(byte cartridgeType) {
	switch (cartridgeType) {
	case 0x00:
		return MapperChip::ROM_ONLY;
	case 0x01:
	case 0x02:
	case 0x03:
		return MapperChip::MBC1;
	case 0x05:
	case 0x06:
		return MapperChip::MBC2;
	case 0x0B:
	case 0x0C:
	case 0x0D:
		return MapperChip::MMM01;
	case 0x0F:
	case 0x11:
		return MapperChip::MBC3;
	case 0x10:
	case 0x12:
	case 0x13:
		return MapperChip::MBC30;
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
		return MapperChip::MBC5;
	case 0x20:
		return MapperChip::MBC6;
	case 0x22:
		return MapperChip::MBC7;
	case 0xFD:
		return MapperChip::TAMA5;
	case 0xFE:
		return MapperChip::HuC3;
	case 0xFF:
		return MapperChip::HuC1;
	default:
		return MapperChip::UNKNOWN;
	}
}

Memory::Memory(rom::RomData&& data)
	: _romData(std::move(data))

	// first rom bank is always stored in rom data, so dont duplicate space
	, _romBanksCart(_romData, MemType::ROM)

	, _ramDataCart(_romData, MemType::RAM)
	, _mapperChip(GetMapperChip(_romData[0x0147]))
{}

byte& Memory::Read(u16 addr) {
	// no mapper chip
	if (_mapperChip == MapperChip::ROM_ONLY && addr < romNEnd)
		return _romData[addr];
	// MBC1
	else if (_mapperChip == MapperChip::MBC1) {
		if (addr < rom0End) {
			if (mbc1::reg::mode == 0)
				return _romBanksCart.Access(0, addr);
				//return _romData[addr];

			// mode 1 behavior
			u8 bankNum = mbc1::reg::bankNum.bank2;
			bankNum <<= 5; // [20-19]: bank2, [18-14]: 0b00000
			return _romBanksCart.Access(bankNum - 1, addr);
		}
		else if (addr < romNEnd) {
			u8 bankNum = mbc1::reg::bankNum();
			return _romBanksCart.Access(bankNum - 1, addr);
		}
	}

	// TODO: all other memory locations
	debug::cexpr::println("Unimplemented or invalid memory access at {:#06x}", addr);
	debug::cexpr::exit(EXIT_FAILURE);
	std::unreachable();
}

void Memory::Write(u16 addr, byte val) {
	if (_mapperChip == MapperChip::MBC1) {
		if (addr < mbc1::ramEnableEnd) {
			val &= 0xF;
			mbc1::reg::ramEnabled = (val == 0b1010);
		}
		else if (addr < mbc1::romBank1End) {
			val = std::max<byte>(val & 0b11111, 1);
			mbc1::reg::bankNum.bank1 = val;
		}
		else if (addr < mbc1::romBank2End) {
			val &= 0b11;
			mbc1::reg::bankNum.bank2 = val;
		}
		else if (addr < mbc1::bankingModeEnd) {
			mbc1::reg::mode = val & 1;
		}
	}
	else {
		debug::cexpr::println("Unimplemented or invalid memory write at {:#06x}", addr);
		debug::cexpr::exit(EXIT_FAILURE);
		std::unreachable();
	}
}

} // namespace gb
