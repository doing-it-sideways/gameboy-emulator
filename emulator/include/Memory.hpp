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
#include "OAM.hpp"

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

	inline bool IsDMAActive() const { return _dmaTransfer.active; }
	void DMATransferTick();

	std::vector<byte> Dump() const; // TODO

	inline auto GetInterruptRegs() { return std::tie(_io.ie, _io.iF); }
	inline InterruptFlags& GetInterruptFlag() { return _io.iF; }

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
	std::array<byte, 0x2000> _vram{};		// video ram -- split into character ram, and bg map data.
	std::array<byte, 0x80> _hram{};			// high ram / zero page.
	std::array<oam::Attribute, 40> _oam{};	// 40 movable objects (sprites), 8x8 or 8x16 pixels
	HWRegs _io;								// io registers
	rom::RomData _romData;					// cartridge rom
	std::vector<byte> _ramInternal;			// no gbc support yet, size always 0x2000

	std::unique_ptr<IMapperInfo> _mapperChipData;
	const MapperChip _mapperChip = MapperChip::UNKNOWN;

	oam::TransferData _dmaTransfer{};

	// Bytes to return in Read when an invalid value needs to be returned
	// Should never be changed, but Read returns a non-const byte&
	static inline constinit std::array<byte, 2> InvalidRead = { 0x00, 0xFF };
};

} // namespace gb

#include "MemoryTemplates.cxx"
