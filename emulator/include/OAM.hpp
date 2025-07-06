#pragma once

#include "Core.hpp"
#include "BitfieldStruct.hpp"

namespace gb {
namespace oam {

BITFIELD_UNION_BYTE(AttribFlags, flags,
	byte CGBPalette : 3;
	byte CGBBank : 1;
	byte DMGPalette : 1;
	byte XFlip : 1;
	byte YFlip : 1;
	byte Priority : 1;
);

#pragma pack(push, 1)
struct Attribute {
	union {
		struct {
			u8 yPos;	// y pos + 16
			u8 xPos;	// x pos + 8
			byte tileIndex;
			AttribFlags attribs;
		};
		byte asBytes[4];
	};
};
#pragma pack(pop)

struct TransferData {
	bool active;
	byte curByte;
	byte srcAddr;

	TransferData(byte writeVal = 0) : active(true), curByte(0), srcAddr(writeVal) {}
};

} // namespace oam
} // namespace gb
