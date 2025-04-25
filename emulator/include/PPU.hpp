#pragma once

#include <array>

#include "Core.hpp"

namespace gb {

class Memory;

namespace ppu {

class GContext {
public:
	explicit GContext(Memory& memory);

private:
	Memory& _memory;
};

} // namespace ppu
} // namespace gb
