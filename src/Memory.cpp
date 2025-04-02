#include "Memory.hpp"
#include <utility>

namespace gb {

Memory::Memory(rom::RomData&& data)
	: _romData(std::move(data))
{
	byte cartridgeType = _romData[0x0147];

	switch (cartridgeType) {
	case 0x00:
		_mapperChip = MapperChip::ROM_ONLY;
		break;
	case 0x01:
	case 0x02:
	case 0x03:
		_mapperChip = MapperChip::MBC1;
		break;
	case 0x05:
	case 0x06:
		_mapperChip = MapperChip::MBC2;
		break;
	case 0x0B:
	case 0x0C:
	case 0x0D:
		_mapperChip = MapperChip::MMM01;
		break;
	case 0x0F:
	case 0x11:
		_mapperChip = MapperChip::MBC3;
		break;
	case 0x10:
	case 0x12:
	case 0x13:
		_mapperChip = MapperChip::MBC30;
		break;
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
		_mapperChip = MapperChip::MBC5;
		break;
	case 0x20:
		_mapperChip = MapperChip::MBC6;
		break;
	case 0x22:
		_mapperChip = MapperChip::MBC7;
		break;
	case 0xFD:
		_mapperChip = MapperChip::TAMA5;
		break;
	case 0xFE:
		_mapperChip = MapperChip::HuC3;
		break;
	case 0xFF:
		_mapperChip = MapperChip::HuC1;
		break;
	default:
		break;
	}
}

} // namespace gb
