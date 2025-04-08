#include "Memory.hpp"
#include <utility>

namespace gb {

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

static struct {
	u8 : 1; // unused high bit
	u8 bank2 : 2 = 0;
	u8 bank1 : 5 = 1;

	byte operator()() { return std::bit_cast<u8>(*this); }
} bankNum;

static bool ramEnabled = false;
static bool mode = false;

} // namespace reg

static constexpr u16 ramEnableEnd = 0x2000;
static constexpr u16 romBank1End = 0x4000;
static constexpr u16 romBank2End = 0x6000;
static constexpr u16 bankingModeEnd = 0x8000;

} // namespace mbc1

static u8 FindRamBanks(byte ramBankCode) {
	switch (ramBankCode) {
	case 2: return 1;
	case 3: return 4;
	case 4: return 16;
	case 5: return 8;
	default: return 0;
	}
}

static Memory::MapperChip FindMapperChip(byte cartridgeType) {
	using MapChip = Memory::MapperChip;

	switch (cartridgeType) {
	case 0x00:
		return MapChip::ROM_ONLY;
	case 0x01:
	case 0x02:
	case 0x03:
		return MapChip::MBC1;
	case 0x05:
	case 0x06:
		return MapChip::MBC2;
	case 0x0B:
	case 0x0C:
	case 0x0D:
		return MapChip::MMM01;
	case 0x0F:
	case 0x11:
		return MapChip::MBC3;
	case 0x10:
	case 0x12:
	case 0x13:
		return MapChip::MBC30;
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
		return MapChip::MBC5;
	case 0x20:
		return MapChip::MBC6;
	case 0x22:
		return MapChip::MBC7;
	case 0xFD:
		return MapChip::TAMA5;
	case 0xFE:
		return MapChip::HuC3;
	case 0xFF:
		return MapChip::HuC1;
	default:
		return MapChip::UNKNOWN;
	}
}

Memory::Memory(rom::RomData&& data)
	: _romData(std::move(data))
	, _ramDataCart()
	, _romBanks(1 << _romData[0x0148])
	, _ramBanks(FindRamBanks(_romData[0x0149]))
	, _mapperChip(FindMapperChip(_romData[0x0147]))
{}

byte& Memory::Read(u16 addr) {
	// no mapper chip
	if (_mapperChip == MapperChip::ROM_ONLY && addr < romNEnd)
		return _romData[addr];
	// MBC1
	else if (_mapperChip == MapperChip::MBC1) {
		if (addr < rom0End)
			return _romData[addr];
		else if (addr < romNEnd) {

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
			val &= 0xF; // only care about low nibble
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
			mbc1::reg::mode = static_cast<bool>(val & 1);
		}
	}

	debug::cexpr::println("Unimplemented or invalid memory write at {:#06x}", addr);
	debug::cexpr::exit(EXIT_FAILURE);
	std::unreachable();
}

} // namespace gb
