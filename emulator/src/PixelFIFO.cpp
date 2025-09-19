#include "PixelFIFO.hpp"

#include "Memory.hpp"
#include "ConstexprAdditions.hpp"

namespace gb::ppu {

PixelFIFO::PixelFIFO(Memory& memory)
	: _memory(memory)
{}

PixelFIFO::FIFOEntry PixelFIFO::Pop() {
	if (_fifo.size() <= 8) {
		debug::cexpr::println(stderr, "FIFO cannot pop!");
		return {};
	}

	FIFOEntry entry = _fifo.front();
	_fifo.pop_front();

	return entry;
}

void PixelFIFO::ResetState() {
	_curXTile = 0;
	_fetchX = 0;
	_curLineX = 0;
	_delay = delayTicks;
	_curMode = FetchMode::GET_TILE;
}

void PixelFIFO::Clear() {
	_fifo.clear();
}

std::optional<PixelFIFO::FIFOEntry> PixelFIFO::Tick(bool evenCol) {
	// PPU fetches first pixel of each line twice
	// faked here with 6 cycle delay (sleep state is ignored)
	if (delayTicks > 0)
		return std::nullopt;

	const u8 scy = _memory[0xFF42];
	const u8 scx = _memory[0xFF43];
	const byte ly = _memory[0xFF44];

	//_curYTile = ((ly + scy) % 8) * 2;

	if (_curMode == FetchMode::PUSH && TryPush8())
		_curMode = FetchMode::GET_TILE;

	if (evenCol)
		FetcherHandler(scy, scx, ly);

	if (_fifo.size() > 8) {
		FIFOEntry entry = Pop();

		// max 12 extra dots of penalty from two tile fetches:
		// first tile in scanline gets discarded (due to scx)
		// + delay at start of line
		if (_curLineX++ >= scx % 8)
			return entry;
	}

	return std::nullopt;
}

u16 PixelFIFO::GetBGWinTilemapStart(LCDControl lcdc) const {
	// TODO: && x coord of cur scanline ! in window
	if ((lcdc.flags.BGTileMapArea && 1) || lcdc.flags.WindowTilemapArea && 1) {
		return altBGTileMapStart; // $9C00
	}
	// TODO: && x coord of cur scanline in window

	return defaultBGTileMapStart; // $9000
}

u16 PixelFIFO::GetTileDataMapAddr(LCDControl lcdc) const {
	// TODO: cgb needs to check vram bank to use & if tile is flipped vertically
	if (lcdc.flags.BGWindowTilemapArea == 1) { // [$8000, $8FFF]
		return altBGWinDataStart + _curTileNum * 0x10;
	}
	else { // [$8800, $97FF]
		sbyte sTileNum = static_cast<sbyte>(_curTileNum);
		return defaultBGWinDataStart + sTileNum * 0x10;
	}
}

bool PixelFIFO::TryPush8() {
	if (_fifo.size() > 8)
		return false;

	// TODO: add paletteNo, bgPrio data
	for (int bit = 7; bit >= 0; --bit) {
		byte colorIndex = (!!((_curDataLo & (1 << bit))) << 1) | (!!(_curDataHi & (1 << bit)));
		_fifo.emplace_back(colorIndex);
	}

	assert(_fifo.size() <= 16);

	return true;
}

void PixelFIFO::FetcherHandler(byte scy, byte scx, byte ly) {
	const auto lcdc = static_cast<LCDControl>(_memory[0xFF40]);

	switch (_curMode) {
	case FetchMode::GET_TILE:
		if (lcdc.flags.BGWindowEnable) {
			u16 bgAddr = GetBGWinTilemapStart(lcdc);

			_curXTile = ((scx / 8) + _fetchX) & 0x1F; // 32 tiles per row
			_curYTile = (ly + scy) & 0xFF;

			bgAddr += _curYTile * 0x20 + _curXTile;

			if (!_memAccessible) [[likely]]
				_curTileNum = _memory[bgAddr];
			else [[unlikely]]
				_curTileNum = 0xFF;
		}

		++_fetchX;
		_curMode = FetchMode::GET_DATA_LOW;
		
		break;

	case FetchMode::GET_DATA_LOW:
		_curDataLo = _memory[GetTileDataMapAddr(lcdc)];
		_curMode = FetchMode::GET_DATA_HIGH;
		
		break;

	case FetchMode::GET_DATA_HIGH:
		_curDataHi = _memory[GetTileDataMapAddr(lcdc) + 1];

		if (_fifo.size() > 8) {
			_curMode = FetchMode::SLEEP;
			break;
		}

		// set mode to push and explicitly fallthrough
		// bg/win pixels attempt to push immediately
		_curMode = FetchMode::PUSH;
		[[fallthrough]];

	case FetchMode::PUSH:
		if (TryPush8())
			_curMode = FetchMode::GET_TILE;

		break;

	case FetchMode::SLEEP:
		_curMode = FetchMode::PUSH;
		break;
	}
}

} // namespace gb::ppu
