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
		debug::cexpr::println("Unknown mapper chip!");
		return {};
	}
}

Memory::Memory(rom::RomData&& data, Timer& timerRegsRef)
	: _io(HWRegs::InitRegs(timerRegsRef))
	, _romData(std::move(data))
	, _ramInternal(0x2000)
	, _mapperChipData(InitMapperChip(_romData[0x0147], _romData[0x0149]))
	, _mapperChip(GetMapperChipType(_romData[0x0147]))
{
	_dmaTransfer.active = false;
}

byte& Memory::Read(u16 addr) {
	// TODO: different behavior for this check on cgb
	// during OAM DMA, cpu can only access HRAM.
	// ppu cannot read OAM properly either
	if (IsDMAActive() && (addr < ioEnd || addr == regIE))
		return InvalidRead[1];

	// [$0000, $7FFF]
	if (addr < romNEnd) {
		if (auto romData = _mapperChipData->ReadRom(*this, addr); romData.has_value())
			return romData.value();
	}
	// [$8000, $9FFF]
	else if (addr < vramEnd) {
		if (_io.stat.flags.PPUMode == 3)
			return InvalidRead[1];

		return _vram[addr - 0x8000];
	}
	// [$A000, $BFFF]
	else if (addr < ramCartEnd) {
		if (auto cartRamData = _mapperChipData->ReadRam(addr); cartRamData.has_value())
			return cartRamData.value();
	}
	// [$C000, $DFFF]
	else if (addr < ramNEnd) {
		// TODO?: cgb has switchable banks (1-7)
		return _ramInternal[addr - 0xC000];
	}
	// [$E000, $FDFF]
	else if (addr < echoRamEnd) {
		// mapped to wram
		return _ramInternal[addr - 0xE000];
	}
	// [$FE00, $FE9F]
	else if (addr < oamEnd) {
		byte mode = _io.stat.flags.PPUMode;
		if ((mode == 2 || mode == 3) && !IsDMAActive())
			return InvalidRead[1];

		addr -= 0xFE00;
		auto& oamData = _oam[addr / 4];
		return oamData.asBytes[addr % 4];
	}
	// [$FEA0, $FEFF]
	else if (addr < unusableEnd) {
		// TODO
		// https://gbdev.io/pandocs/Memory_Map.html#fea0feff-range
	}
	// [$FF00, $FF7F]
	else if (addr < ioEnd) {
		return _io.Read(addr);
	}
	// [$FF80, $FFFE]
	else if (addr < hramEnd) {
		return _hram[addr - 0xFF80];
	}
	// $FFFF
	else {
		return _io.ie.asByte;
	}

	debug::cexpr::println("Unimplemented or invalid memory access at {:#06x}", addr);
	debug::cexpr::exit(EXIT_FAILURE);
	std::unreachable();
}

void Memory::Write(u16 addr, byte val) {
	// TODO: different behavior for this check on cgb
	if (IsDMAActive() && (addr < ioEnd || addr == regIE))
		return;

	if (addr < romNEnd) {
		if (_mapperChipData->AttemptWriteRam(addr, val))
			return;
	}
	// [$8000, $9FFF]
	else if (addr < vramEnd) {
		if (_io.stat.flags.PPUMode == 3)
			return;

		_vram[addr - 0x8000] = val;
		return;
	}
	// [$A000, $BFFF]
	else if (addr < ramCartEnd) {
		if (_mapperChipData->AttemptWriteRam(addr, val))
			return;
	}
	// [$C000, $DFFF]
	else if (addr < ramNEnd) {
		// TODO?: cgb has switchable banks (1-7)
		_ramInternal[addr - 0xC000] = val;
		return;
	}
	// [$E000, $FDFF]
	else if (addr < echoRamEnd) {
		// mapped to wram
		_ramInternal[addr - 0xE000] = val;
		return;
	}
	// [$FE00, $FE9F]
	else if (addr < oamEnd) {
		byte mode = _io.stat.flags.PPUMode;
		if ((mode == 2 || mode == 3) && !IsDMAActive())
			return;

		addr -= 0xFE00;
		auto& oamData = _oam[addr / 4];
		oamData.asBytes[addr % 4] = val;
	}
	// [$FEA0, $FEFF]
	else if (addr < unusableEnd) {
		// TODO
		// https://gbdev.io/pandocs/Memory_Map.html#fea0feff-range
	}
	// [$FF00, $FF7F]
	else if (addr < ioEnd) {
		if (addr == 0xFF46)
			_dmaTransfer = oam::TransferData(val); // reset transfer state to be on

		_io.Write(addr, val);
		return;
	}
	// [$FF80, $FFFE]
	else if (addr < hramEnd) {
		_hram[addr - 0xFF80] = val;
		return;
	}
	// $FFFF
	else {
		_io.ie = val;
		return;
	}
	
	debug::cexpr::println("Unimplemented or invalid memory write at {:#06x}", addr);
	debug::cexpr::exit(EXIT_FAILURE);
	std::unreachable();
}

void Memory::DMATransferTick() {
	assert(_dmaTransfer.active);

	// writes from $FE00 to $FE9F
	u16 dmaSrcAddr = _dmaTransfer.srcAddr * 0x100;
	Write(0xFE00 + _dmaTransfer.curByte, Read(dmaSrcAddr + _dmaTransfer.curByte));

	if (++_dmaTransfer.curByte == 0xA0)
		_dmaTransfer.active = false;
}

} // namespace gb
