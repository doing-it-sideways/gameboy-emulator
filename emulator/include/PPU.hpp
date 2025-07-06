#pragma once

#include <array>
#include <glm/vec3.hpp>

#include "Core.hpp"
#include "HardwareRegisters.hpp"

namespace gb {

class Memory;

namespace ppu {

enum class Mode : byte {
	HBLANK = 0,
	VBLANK = 1,
	OAM_SCAN = 2,
	PIXEL_DRAW = 3
};

enum class Palette : byte {
	BGP = 0,
	OBP0 = 1,
	OBP1 = 2
};

class GContext {
public:
	static constexpr std::array<glm::vec3, 4> dmgColors = {
		glm::vec3{ 1 },
		glm::vec3{ .6666f },
		glm::vec3{ .3333f },
		glm::vec3{ 0 }
	};

public:
	explicit GContext(Memory& memory);

	bool Update();

	Mode GetMode() const;
	void SetMode(Mode newMode);

	PaletteData GetPaletteData(Palette palette) const;
	void SetPaletteData(Palette palette, PaletteData newIndices);

private:
	void UpdateLine(byte& ly);

private:
	static constexpr u16 lineMax = 153;
	static constexpr u16 vBlankStartNext = 143;
	static constexpr u16 dotsPerLine = 456;

	static constexpr u16 hBlankMaxDots = 376 - 172;
	static constexpr u16 oamScanDots = 80;
	static constexpr u16 pixelDrawMaxDots = 289;

	Memory& _memory;

	//u16 _curLine = 0; use ly register
	u16 _curDot = 0;
};

} // namespace ppu
} // namespace gb
