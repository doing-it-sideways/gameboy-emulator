#include "MemoryBank.hpp"

namespace gb {

static constexpr u64 GetDataSize(MemType type, u8 banks) {
	switch (type) {
	case MemType::ROM:
	case MemType::ROM_MULTICART:
		return static_cast<u64>(romBankSize) * banks;
	case MemType::RAM:
		return static_cast<u64>(ramBankSize) * banks;
	default:
		std::unreachable();
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

static MemType IsRomMulticart(const rom::RomData& data) {
	// TODO -- detect mbc1 multicarts
	return MemType::ROM;
}

MemoryBank::MemoryBank(rom::RomData& fullMem, MemType type)
	//: _data(GetDataSize(type, banks))
	: _romData(fullMem)
	, _type(type == MemType::RAM ? type : IsRomMulticart(fullMem))
	, _banks(type == MemType::RAM ? GetRamBanks(fullMem[0x0149]) : 2 << fullMem[0x0148])
{}

byte& MemoryBank::GetRom(u8 bank, u16 addr) {
	// bank num [20-14] | addr [13-0]
	u32 physicalAddr = (static_cast<u32>(bank) << 14) | (addr & 0x3FFF);

	return _romData[physicalAddr];
}

byte& MemoryBank::GetRam(u8 bank, u16 addr) {
	// TODO
	return _romData[0];
}

} // namespace gb
