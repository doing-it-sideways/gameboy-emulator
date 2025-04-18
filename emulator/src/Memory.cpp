#include <utility>

#include "Memory.hpp"
#include "ConstexprAdditions.hpp"

namespace gb {

static MapperChip GetMapperChipType(byte type) {
	switch (type) {
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

// Determine if the MBC1 cartridge is a multicart or not.
static std::unique_ptr<IMapperInfo> GetMBC1CartType(byte ramSizeCode = 0) {
	// TODO -- see MemoryBank.cpp IsROMMulticart
	return std::make_unique<MBC1>(ramSizeCode);
}

static std::unique_ptr<IMapperInfo> InitMapperChip(byte type, byte ramSizeCode) {
	MapperChip mc = GetMapperChipType(type);

	switch (type) {
	case 0x00:
		return std::make_unique<NoMBC>();
	case 0x01:
		return GetMBC1CartType();
	case 0x02:
	case 0x03:
		return GetMBC1CartType(ramSizeCode);
	//case 0x05: // weird case
	//case 0x06: // weird case
	case 0x08: // no licensed cartridge uses this. behavior unknown
	case 0x09: // no licensed cartridge uses this. behavior unknown
		return std::make_unique<NoMBC>(ramSizeCode);
	//case 0x0C:
	//case 0x0D:
	//case 0x10:
	//case 0x12:
	//case 0x13:
	//case 0x1A:
	//case 0x1B:
	//case 0x1D:
	//case 0x1E:
	//case 0x20: // weird case
	//case 0x22:
	//case 0xFE:
	//case 0xFF:
	default:
		debug::cexpr::println("Unknown mapper chip! Things might break...");
		return {};
	}
}

Memory::Memory(rom::RomData&& data)
	: _romData(std::move(data))
	, _mapperChipData(InitMapperChip(_romData[0x0147], _romData[0x0149]))
	, _ramInternal(0x2000)
	, _hram()
	, _romBanksCart(_romData, MemType::ROM)
	, _ramDataCart(_romData, MemType::RAM)
	, _mapperChip(GetMapperChipType(_romData[0x0147]))
{}

byte& Memory::Read(u16 addr) {
	if (addr >= 0xA000 && addr < 0xC000) {
		// TODO: dont return ref to local
		auto data = _mapperChipData->ReadRam(addr);

		if (data.has_value())
			return data.value();
	}

	// attempt to read data from a rom bank. all rom banks are stored on the cartridge
	// addresses: [0x0000, 0x7FFF]
	if (auto romData = _mapperChipData->ReadRom(_romBanksCart, addr); romData.has_value())
		return romData.value();

	// attempt to read ram data from cartridge ram
	// addresses: [0xA000, 0xBFFF]
	if (auto ramData = _mapperChipData->ReadRam(addr); ramData.has_value())
		return ramData.value();

	// TODO: all other memory locations
	debug::cexpr::println("Unimplemented or invalid memory access at {:#06x}", addr);
	debug::cexpr::exit(EXIT_FAILURE);
	std::unreachable();
}

void Memory::Write(u16 addr, byte val) {
	// attempt to write data to cartridge ram
	// addresses: [0xA000, 0xBFFF]
	if (_mapperChipData->AttemptWriteRam(addr, val))
		return;

	// attempt to write data to internal ram
	// addresses: [0xC000, 0xDFFF]
	if (addr >= ramCartEnd && addr < ramNEnd) {
		_ramInternal[addr - 0xC000] = val;
		return;
	}
	
	debug::cexpr::println("Unimplemented or invalid memory write at {:#06x}", addr);
	debug::cexpr::exit(EXIT_FAILURE);
	std::unreachable();
}

} // namespace gb
