#pragma once

#include <array>
#include <deque>
#include <optional>

#include "Core.hpp"
#include "HardwareRegisters.hpp"

namespace gb {

class Memory;

namespace ppu {

/*
	Pixel FIFO:
		- Another state machine
		- Two FIFOs
			- BG pixels
			- Object pixels
			- not shared & independent. Mixed only when popping items
				- Objects take priority unless they're transparent (color 0)
		- Holds 16 pixels
		- FIFO & pixel fetcher work together, ensure FIFO always contains min 8 pixels
			- 8 pixels required for pixel rendering operation to take place
		- Only manipulated during mode 3
		- 4 members per entry:
			- Color (0-3)
			- Palette (0-7) [CGB: everything, DMG: Objects only]
			- Sprite prio: [CGB: OAM index for the object, DMG: Doesn't exist]
			- BG prio: value of the OBJ-to-BG Priority bit (OAM.7)
*/

/*
	Credit to "The Ultimate Game Boy Talk" and Low Level Devel on YouTube
	for the tutorials with this part, otherwise I couldn't have figured
	it out on my own:
	https://www.youtube.com/watch?v=HyzD8pNlpwI
	https://www.youtube.com/watch?v=RMLIp_pJVv8
*/

enum class FetchMode : byte {
	GET_TILE = 0,
	GET_DATA_LOW = 1,
	GET_DATA_HIGH = 2,
	SLEEP = 3,
	PUSH = 4
};

class PixelFIFO {
public:
	struct FIFOEntry {
		byte colorIndex = 0;
		byte paletteNo = 0;
		bool bgPrio = 0;

		byte objPrio = 0; // CGB only
	};

public:
	explicit PixelFIFO(Memory& memory);

	FIFOEntry Pop();		// returns color of pixel popped
	void Clear();

	std::optional<FIFOEntry> Tick(bool evenCol);
	void ResetState();

private:
	// Returns either 0x9800 or 0x9C00 depending on flags set in the lcdc register.
	u16 GetBGWinTilemapStart(LCDControl lcdc) const;
	u16 GetTileDataMapAddr(LCDControl lcdc) const;

	// Converts 2 bytes of fetched data into 8 fifo entries and pushes them to the fifo.
	bool TryPush8();

	void FetcherHandler(byte scy, byte scx, byte ly);

private:
	static constexpr u16 defaultBGTileMapStart = 0x9800;
	static constexpr u16 altBGTileMapStart = 0x9C00;
	static constexpr u16 defaultBGWinDataStart = 0x9000;
	static constexpr u16 altBGWinDataStart = 0x8000;

	static constexpr byte maxFIFOEntries = 16;
	static constexpr byte delayTicks = 6;

	Memory& _memory;

	std::deque<FIFOEntry> _fifo{};
	
	u16 _fetchX = 0, _curLineX = 0;
	byte _curXTile = 0, _curYTile = 0;

	byte _curTileNum = 0;
	byte _curDataLo = 0, _curDataHi = 0;

	byte _delay = delayTicks;
	FetchMode _curMode = FetchMode::GET_TILE;

	bool _memAccessible = false;
};

} // namespace ppu
} // namespace gb
