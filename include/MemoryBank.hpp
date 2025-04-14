#pragma once

#include <vector>
#include <stdexcept>

#include "Core.hpp"
#include "ROM.hpp"

namespace gb {

enum class MemType : byte {
	ROM,
	ROM_MULTICART,
	RAM
};
// TODO: make this an interface -- one impl for rom, one for ram
class MemoryBank {
public:
	MemoryBank(rom::RomData& fullMem, MemType type);

	// TODO: VS 17.14, change back to operator[]. intellisense breaks all syntax highlighting
	// because they forgot to add support for templated multidimensional subscript operator.
	template <typename Self>
	//auto&& operator[](this Self&& self, u8 bank, u16 addr);
	auto&& Access(this Self&& self, u8 bank, u16 addr);

	inline u8 Banks() const { return _banks; }

	std::vector<byte> Dump() const; // TODO

private:
	byte& GetRom(u8 bank, u16 addr);
	byte& GetRam(u8 bank, u16 addr);

private:
	rom::RomData& _romData;
	const MemType _type;
	const u8 _banks;
};

} // namespace gb

#include "MemoryBankTemplates.cxx"
