#pragma once

#include "Core.hpp"
#include "ROM.hpp"

namespace gb {

class Memory {
public:
	Memory(rom::RomData&& data);

	// C++23 features that allow explicit object params to handle const/non-const in one func
	auto&& operator[](this auto&& self, u16 addr);

private:

private:
	rom::RomData romData;
};

} // namespace gb

#include "MemoryTemplates.cxx"
