#pragma once

#include "Core.hpp"
#include "ROM.hpp"
#include "ConstexprAdditions.hpp"

namespace gb {

class Memory {
public:
	// Same as RomData but name for clarity
	using MemData = std::vector<byte>;

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

public:
	Memory(rom::RomData&& data);

	template <typename Self>
	auto operator[](this Self&& self, u16 addr);

	byte& Read(u16 addr);
	void Write(u16 addr, byte val);

	MemData Dump() const; // TODO

private:
	// Allows operator[] to work properly
	class Accessor {
	public:
		Accessor(Memory& base, u16 addr) : _base(base), _addr(addr) {}

		// Handles reading data
		byte& operator()() const { return _base.Read(_addr); }
		operator byte&() const { return this->operator()(); }

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

	MemData _ramDataCart; // cartridge ram

	const u16 _romBanks = 0;  // rom banks on the cartridge
	const u8 _ramBanks = 0; // ram banks on the cartridge
	const MapperChip _mapperChip = MapperChip::UNKNOWN;
};

} // namespace gb

#include "MemoryTemplates.cxx"
