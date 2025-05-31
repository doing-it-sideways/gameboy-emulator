#pragma once

#include <array>
#include <tuple>
#include <memory>
#include <vector>

#include "Core.hpp"
#include "MapperChipInfo.hpp"
#include "ROM.hpp"
#include "HardwareRegisters.hpp"
#include "Timer.hpp"

namespace gb {

class Memory {
public:
	// TODO: have hwregs live in memory and make just the timer live in emu?
	explicit Memory(rom::RomData&& data, Timer& timerRegsRef);

	template <typename Self>
	auto operator[](this Self&& self, u16 addr);

	byte& Read(u16 addr);

	// Used by mapper chips to avoid infinite indirect recursion.
	inline byte& ReadRom(u16 physicalAddr) { return _romData[physicalAddr]; }

	void Write(u16 addr, byte val);

	std::vector<byte> Dump() const; // TODO

	inline auto GetInterruptReg() { return std::tie(_io.ie, _io.iF); }

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
	HWRegs _io;							// io registers
	rom::RomData _romData;				// cartridge rom
	std::vector<byte> _ramInternal;		// no gbc support yet, size always 0x2000

	std::unique_ptr<IMapperInfo> _mapperChipData;

	const MapperChip _mapperChip = MapperChip::UNKNOWN;
};

} // namespace gb

#include "MemoryTemplates.cxx"
