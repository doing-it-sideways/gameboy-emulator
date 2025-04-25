#include "PPU.hpp"
#include "Memory.hpp"

namespace gb::ppu {

GContext::GContext(Memory& memory)
	: _memory(memory)
{}

} // namespace gb::ppu
