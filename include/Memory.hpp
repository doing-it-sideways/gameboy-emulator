#pragma once

#include "Core.hpp"
#include "ROM.hpp"

namespace gb {

class Memory {
public:
	constexpr Memory(rom::RomData&& data);

	constexpr byte operator[](u16 address);

private:
	rom::RomData romData;
};

} // namespace gb
