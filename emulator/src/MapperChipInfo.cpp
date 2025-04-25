#include "MapperChipInfo.hpp"
#include "Memory.hpp"

namespace gb {

static u32 RamSize(byte code) {
	constexpr u32 bankSize = 8192;

	switch (code) {
	case 2: return bankSize;
	case 3: return 4 * bankSize;
	case 4: return 16 * bankSize;
	case 5: return 8 * bankSize;
	default: return 0;
	}
}

IMapperInfo::IMapperInfo(byte ramSizeCode)
	: _ramAvailable(ramSizeCode != 0)
	, _ram(RamSize(ramSizeCode))
{}

NoMBC::NoMBC(byte ramSizeCode)
	: IMapperInfo(ramSizeCode)
{}

OptByteRef NoMBC::ReadRom(Memory& mem, u16 addr) {
	// TODO
	if (addr < romNEnd)
		return mem.ReadRom(addr);

	return {};
}

bool NoMBC::AttemptWriteRam(u16 addr, byte val) {
	return false; // TODO
}

OptByteRef NoMBC::ReadRam(u16 addr) {
	return {}; // TODO
}

MBC1::MBC1(byte ramSizeCode)
	: IMapperInfo(ramSizeCode)
{}

// 0xx yyyyy
//  ^ bank2
//     ^ bank1
byte MBC1::Bank(u16 addr) const {
	if (addr < rom0End)
		return (_mode == 0) ? 0 : _bank2 << 5;

	return _bank2 << 5 | _bank1;
}

OptByteRef MBC1::ReadRom(Memory& mem, u16 addr) {
	if (addr < romNEnd)
		return mem.ReadRom((Bank(addr) << 14) | (addr & 0x3FFF));

	return {};
}

bool MBC1::AttemptWriteRam(u16 addr, byte val) {
	if (addr < ramEnableEnd) {
		val &= 0xF;
		_ramEnabled = (val == 0b1010);
		return true;
	}
	else if (addr < romBank1End) {
		val = std::max<byte>(val & 0b11111, 1);
		_bank1 = val;
		return true;
	}
	else if (addr < romBank2End) {
		_bank2 = val & 0b11;
		return true;
	}
	else if (addr < bankingModeEnd) {
		_mode = val & 1;
		return true;
	}

	if (!_ramAvailable || addr < vramEnd || addr >= ramCartEnd)
		return false;

	// TODO

	return true;
}

OptByteRef MBC1::ReadRam(u16 addr) {
	if (!_ramAvailable || addr < vramEnd || addr >= ramCartEnd)
		return {};

	u16 physicalAddr = static_cast<u16>(_bank2) << 13 | (addr & 0x1FFF);
	return _ram[physicalAddr];
}

} // namespace gb
