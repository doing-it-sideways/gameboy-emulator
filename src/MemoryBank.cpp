#include "MemoryBank.hpp"

namespace gb {

// 1 << (1 + total bits used for address)
static constexpr u64 romBankSize = 1 << 14;
static constexpr u64 ramBankSize = 1 << 13;

static u64 GetDataSize(MemType type, u8 banks) {
	switch (type) {
	case MemType::ROM: return romBankSize * banks;
	case MemType::RAM: return ramBankSize * banks;
	default: std::unreachable();
	}
}

MemoryBank::MemoryBank(MemType type, u8 banks)
	: _data(GetDataSize(type, banks))
	, _type(type)
	, _banks(banks)
{}

byte& MemoryBank::GetRom(u8 bank, u16 addr) {
	return _data[romBankSize * bank + addr];
}

byte& MemoryBank::GetRam(u8 bank, u16 addr) {
	return _data[ramBankSize * bank + addr];
}


} // namespace gb
