#pragma once

#include "Core.hpp"
#include "ROM.hpp"
#include "ConstexprAdditions.hpp"

namespace gb {

class Memory {
	// Allows operator[] to work properly
	class Accessor {
	public:
		constexpr Accessor(Memory& base, u16 addr) : _base(base), _addr(addr) {}

		// Handles reading data
		constexpr byte& operator()() { return _base.Read(_addr); }
		constexpr operator byte&() { return this->operator()(); }

		// Handles writing data
		constexpr Accessor& operator=(byte data) { _base.Write(_addr, data); return *this; }

	private:
		Memory& _base;
		u16 _addr;
	};

public:
	Memory(rom::RomData&& data);

	template <typename Self>
	constexpr auto operator[](this Self&& self, u16 addr);

	constexpr byte& Read(u16 addr);
	constexpr void Write(u16 addr, byte val);

private:
	enum class MapperChip : byte {
		ROM_ONLY = 0,
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
		TAMA5,
		UNKNOWN = 255
	};

private:
	rom::RomData _romData;

	u16 _extraROMBanks = 0;
	MapperChip _mapperChip = MapperChip::UNKNOWN;
};

} // namespace gb

#include "MemoryTemplates.cxx"
