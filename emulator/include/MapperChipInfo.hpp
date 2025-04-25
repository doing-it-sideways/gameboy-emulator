#pragma once

#include <functional>
#include <optional>
#include <vector>

#include "Core.hpp"

/*
	The chip register names are taken from gekkio.fi's gameboy complete
	technical reference. Writing values to these registers doesn't actually write
	to memory; they instead change behavior in the chip. Details of what they do are
	in the technical reference, chapter 12.
*/

namespace gb {

enum class MapperChip : byte {
	UNKNOWN,
	ROM_ONLY,
	MBC1,
	MBC1_MULTI,
	MBC2,
	MBC3,
	MBC30,
	MBC5,
	MBC6,
	MBC7,
	HuC1,
	HuC3,
	MMM01,
	TAMA5
};

class Memory;
using OptByteRef = std::optional<std::reference_wrapper<byte>>;

struct IMapperInfo {
	explicit IMapperInfo(byte ramSizeCode = 0);
	virtual ~IMapperInfo() {}

	constexpr IMapperInfo(IMapperInfo&&) = default;

	// Bank number retrieval method
	virtual byte Bank(u16 addr) const = 0;

	// Each cartridge has a specific way of reading rom values. Let the 
	virtual OptByteRef ReadRom(Memory& mem, u16 addr) = 0;
	
	// Updates cartridge-specific registers or writes to
	// cartridge ram (if present).
	// Ret: true -- if a register was updated or cartridge ram was written to.
	//		false -- if no valid register was at the adress, or ram is disabled
	virtual bool AttemptWriteRam(u16 addr, byte val) = 0;

	// Attempts to read a value from cartridge ram.
	// Ret: byte value if ram is enabled. no value if ram is disabled
	virtual OptByteRef ReadRam(u16 addr) = 0;

	const bool _ramAvailable;

protected:
	std::vector<byte> _ram;
};

class NoMBC : public IMapperInfo {
public:
	explicit NoMBC(byte ramSizeCode = 0);

	byte Bank(u16 addr) const override { return 0; }

	OptByteRef ReadRom(Memory& mem, u16 addr) override;

	bool AttemptWriteRam(u16 addr, byte val);

	OptByteRef ReadRam(u16 addr) override;
};

class MBC1 : public IMapperInfo {
public:
	explicit MBC1(byte ramSizeCode = 0);

	byte Bank(u16 addr) const override;

	OptByteRef ReadRom(Memory& mem, u16 addr) override;

	bool AttemptWriteRam(u16 addr, byte val) override;

	OptByteRef ReadRam(u16 addr) override;

	inline bool RamEnabled() const { return _ramEnabled; }
	inline byte Mode() const { return _mode; }

private:
	static constexpr u16 ramEnableEnd = 0x2000;
	static constexpr u16 romBank1End = 0x4000;
	static constexpr u16 romBank2End = 0x6000;
	static constexpr u16 ramBankEnd = romBank2End;
	static constexpr u16 bankingModeEnd = 0x8000;

	u8 : 1;
	u8 _bank2 : 2 = 0;
	u8 _bank1 : 5 = 1;

	bool _ramEnabled = false;
	byte _mode = 0;
};

//struct MBC1Multi : public IMapperInfo {};
//struct MBC2 : public IMapperInfo {};
//struct MBC3 : public IMapperInfo {};
//struct MBC5 : public IMapperInfo {};
//struct MBC6 : public IMapperInfo {};
//struct MBC6 : public IMapperInfo {};
//struct MBC7 : public IMapperInfo {};
//struct HuC1 : public IMapperInfo {};
//struct HuC3 : public IMapperInfo {};
//struct MMM01 : public IMapperInfo {};
//struct TAMA5 : public IMapperInfo {};

} // namespace gb
