#pragma once

#include "Core.hpp"
#include "ROM.hpp"
#include "ConstexprAdditions.hpp"

namespace gb {

class Memory {
public:
	Memory(rom::RomData&& data);

	// C++23 feature that allows explicit object params, which in this case,
	// handles const/non-const in one function. Downside is because of auto,
	// it has to be inlined, so implementation is included in .cxx
	constexpr auto&& operator[](this auto&& self, u16 addr);

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
		UNKNOWN = ~0
	};

private:
	rom::RomData _romData;
	MapperChip _mapperChip = MapperChip::UNKNOWN;
};

} // namespace gb

#include "MemoryTemplates.cxx"
