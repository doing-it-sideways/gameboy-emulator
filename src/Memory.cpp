#include "Memory.hpp"
#include <utility>

namespace gb {

Memory::Memory(rom::RomData&& data)
	: romData(std::move(data))
{
	// TODO
}

} // namespace gb
