#pragma once

#include <vector>

#include "Core.hpp"
#include "MemoryBank.hpp"
#include "ROM.hpp"

namespace gb {

enum class MapperChip : byte {
	UNKNOWN,
	ROM_ONLY,
	MBC1,
	MBC2,
	MBC3,
	MBC30,
	MBC4,
	MBC5,
	MBC6,
	MBC7,
	HuC1,
	HuC3,
	MMM01,
	TAMA5
};

class Memory {
public:
	Memory(rom::RomData&& data);

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

	MemoryBank _romBanksCart; // extra cartridge rom
	MemoryBank _ramDataCart;  // cartridge ram

	const MapperChip _mapperChip = MapperChip::UNKNOWN;
};

} // namespace gb

#include "MemoryTemplates.cxx"
