#pragma once

#include <vector>
#include <stdexcept>

#include "Core.hpp"

namespace gb {

enum class MemType : byte {
	ROM,
	RAM
};

class MemoryBank {
public:
	MemoryBank(MemType type, u8 banks = 1);

	// TODO: VS 17.14, change back to operator[]. intellisense breaks all syntax highlighting
	// because they forgot to add support for templated multidimensional subscript operator.
	template <typename Self>
	//auto&& operator[](this Self&& self, u8 bank, u16 addr);
	auto&& Access(this Self&& self, u8 bank, u16 addr);

	inline u8 Banks() const { return _banks; }

private:
	byte& GetRom(u8 bank, u16 addr);
	byte& GetRam(u8 bank, u16 addr);

private:
	std::vector<byte> _data;
	const MemType _type;
	const u8 _banks;
};

} // namespace gb

#include "MemoryBankTemplates.cxx"
