#pragma once

#include <array>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

#include "Core.hpp"
#include "HardwareRegisters.hpp"

namespace gb {

class Memory;

namespace ppu {

/*
	VRAM Tile Data:
		- 384 tiles
		- Objects: [$8000, $8FFF]
		- BG/Window:
			LCDC.4 == 1: [$8000, $8FFF]
			LCDC.4 == 0: [$8800, $97FF] -> $9000 == 0, $8800 == -128
		- Palette has 4 colors: white -> light gray -> dark gray, black
			- Objects "white" is transparent
		- Line of pixels: two bytes, combined to show all colors
			- $3C $7E: 00111100 (<-FLIP->) 01111110: 00, 10, 11, 11, 11, 10, 00
	PPU:
		- 4 Modes, State Machine
		- Formats output during mode 3 (drawing pixels)
		- Restricts VRAM access during mode 3
		- Restricts OAM access during modes 2 (OAM scan) and 3
		- Mode 3 length is variable:
			- Min 172 dots, start at 289 dots.
			- Penalize draw when: background scrolling (SCX % 8),
			  last non-window pixel emitted before window drawing (6),
			  drawing objects (6-11)
		- Mode 0 length = 376 - Mode 3 length
	Pixel FIFO:
		- Two FIFOs
			- BG pixels
			- Object pixels
			- not shared & independent. Mixed only when popping items
				- Objects take priority unless they're transparent (color 0)
		- Holds 16 pixels
		- FIFO & pixel fetcher work together, ensure FIFO always contains min 8 pixels
			- 8 pixels required for pixel rendering operation to take place
		- Only manipulated during mode 3
		- 4 members:
			- Color (0-3)
			- Palette (0-7) [CGB: everything, DMG: Objects only]
			- Sprite prio: [CGB: OAM index for the object, DMG: Doesn't exist]
			- BG prio: value of the OBJ-to-BG Priority bit (OAM.7)
*/

// Used in Emulator.cpp. If end of frame, wait to hit target FPS of 60.
enum class State : byte {
	PROCESSING,
	END_FRAME
};

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
	static constexpr std::array<glm::vec4, 4> dmgColors = {
		glm::vec4{ 1.f },
		glm::vec4{ glm::vec3{ .6666f }, 1.f },
		glm::vec4{ glm::vec3{ .3333f }, 1.f },
		glm::vec4{ glm::vec3{ 0.f }, 1.f }
	};

public:
	explicit GContext(Memory& memory);

	State Update();

	Mode GetMode() const;
	void SetMode(Mode newMode);

	PaletteData GetPaletteData(Palette palette) const;
	void SetPaletteData(Palette palette, PaletteData newIndices);

private:
	void UpdateLine(byte& ly);

private:
	static constexpr u16 lineMax = 153;
	static constexpr u16 vBlankStart = 144;
	static constexpr u16 dotsPerLine = 456;

	static constexpr u16 hBlankMaxDots = 376 - 172;
	static constexpr u16 oamScanDots = 80;
	static constexpr u16 pixelDrawMaxDots = 289;

	Memory& _memory;

	//u16 _curLine = 0; use ly register
	u16 _curDot = 0;

	// The amount of dots to draw during mode 3. Range: [172, 289]
	u16 _pixelDrawDots = pixelDrawMaxDots;
};

} // namespace ppu
} // namespace gb
