#pragma once

#include <array>
#include <memory>
#include <vector>

#include "Core.hpp"
#include "MapperChipInfo.hpp"
#include "MemoryBank.hpp"
#include "ROM.hpp"

namespace gb {

class Memory {
public:
	explicit Memory(rom::RomData&& data);

	template <typename Self>
	auto operator[](this Self&& self, u16 addr);

	byte& Read(u16 addr);
	void Write(u16 addr, byte val);

	std::vector<byte> Dump() const; // TODO

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
	// TODO: funcs for setting up rom/ram values/registers/etc

private:
	rom::RomData _romData;
	std::vector<byte> _ramInternal; // no gbc support yet, size always 0x2000
	std::array<byte, 0x80> _hram; // high ram / zero page. 

	std::unique_ptr<IMapperInfo> _mapperChipData;

	MemoryBank _romBanksCart; // extra cartridge rom
	MemoryBank _ramDataCart;  // cartridge ram

	const MapperChip _mapperChip = MapperChip::UNKNOWN;
};

} // namespace gb

#include "MemoryTemplates.cxx"
