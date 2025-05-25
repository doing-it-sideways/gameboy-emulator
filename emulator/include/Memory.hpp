#pragma once

#include <array>
#include <memory>
#include <vector>

#include "Core.hpp"
#include "MapperChipInfo.hpp"
#include "ROM.hpp"

namespace gb {

class Memory {
public:
	explicit Memory(rom::RomData&& data);

	template <typename Self>
	auto operator[](this Self&& self, u16 addr);

	byte& Read(u16 addr);

	// Used by mapper chips to avoid infinite indirect recursion.
	inline byte& ReadRom(u16 physicalAddr) { return _romData[physicalAddr]; }

	void Write(u16 addr, byte val);

	std::vector<byte> Dump() const; // TODO

	inline byte GetIE() const { return _ie; }

private:
	// Allows operator[] to work properly
	class Accessor {
	public:
		Accessor(Memory& base, u16 addr) : _base(base), _addr(addr) {}

		// Handles reading data
		byte& operator()() const { return _base.Read(_addr); }
		operator byte&() const { return _base.Read(_addr); }

		// Handles writing data
		Accessor& operator=(byte data) { _base.Write(_addr, data); return *this; }

	private:
		Memory& _base;
		u16 _addr;
	};

private:	
	std::array<byte, 0x1800> _vram{};	// video ram
	std::array<byte, 0x80> _hram{};		// high ram / zero page.
	byte _ie = 0;						// Interrupt enable flag

	rom::RomData _romData;			// cartridge rom
	std::vector<byte> _ramInternal; // no gbc support yet, size always 0x2000

	std::unique_ptr<IMapperInfo> _mapperChipData;

	const MapperChip _mapperChip = MapperChip::UNKNOWN;
};

} // namespace gb

#include "MemoryTemplates.cxx"
