#pragma once

#include "Core.hpp"
#include "ROM.hpp"

namespace gb {

class Memory {
public:
	Memory(rom::RomData&& data);

	// C++23 feature that allows explicit object params, which in this case,
	// handles const/non-const in one function. Downside is because of auto,
	// it has to be inlined, so implementation is included in .cxx
	auto&& operator[](this auto&& self, u16 addr);

private:

private:
	rom::RomData romData;
};

} // namespace gb

#include "MemoryTemplates.cxx"
