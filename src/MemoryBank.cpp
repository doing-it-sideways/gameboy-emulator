#include "MemoryBank.hpp"

namespace gb {

// 1 << (1 + total bits used for address)
static constexpr u64 romBankSize = 1 << 14;
static constexpr u64 ramBankSize = 1 << 13;

static constexpr u64 GetDataSize(MemType type, u8 banks) {
	switch (type) {
	case MemType::ROM: return romBankSize * banks;
	case MemType::RAM: return ramBankSize * banks;
	default: std::unreachable();
	}
}

static constexpr u8 GetRamBanks(byte ramBankCode) {
	switch (ramBankCode) {
	case 2: return 1;
	case 3: return 4;
	case 4: return 16;
	case 5: return 8;
	default: return 0;
	}
}

MemoryBank::MemoryBank(rom::RomData& fullMem, MemType type)
	//: _data(GetDataSize(type, banks))
	: _fullMem(fullMem)
	, _type(type)
	, _banks(type == MemType::ROM ? 2 << fullMem[0x0148] : GetRamBanks(fullMem[0x0149]))
{}

byte& MemoryBank::GetRom(u8 bank, u16 addr) {
	// bank num [20-14] | addr [13-0]
	u32 physicalAddr = (static_cast<u32>(bank) << 14) | (addr & 0x3FFF);

	return _fullMem[physicalAddr];
}

byte& MemoryBank::GetRam(u8 bank, u16 addr) {
	// TODO
	return _fullMem[0];
}


} // namespace gb
