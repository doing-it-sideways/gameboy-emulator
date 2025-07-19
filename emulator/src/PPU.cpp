#include "PPU.hpp"
#include "Memory.hpp"

namespace gb::ppu {

GContext::GContext(Memory& memory)
	: _memory(memory)
{
	SetMode(Mode::OAM_SCAN);
}

State GContext::Update() {
	// ly should only ever be updated inside the ppu
	byte& ly = _memory[0xFF44];

	// state machine
	switch (GetMode()) {
	// TODO: HBLANK
	case Mode::HBLANK:
		if (_curDot != dotsPerLine) {
			++_curDot;
			break;
		}

		UpdateLine(ly);
		_curDot = 0;

		if (ly == vBlankStart) {
			SetMode(Mode::VBLANK);
			return State::END_FRAME;
		}
		else
			SetMode(Mode::OAM_SCAN);

		break;

	case Mode::VBLANK:
		if (_curDot != dotsPerLine) {
			++_curDot;
			break;
		}

		if (ly == lineMax) {
			SetMode(Mode::OAM_SCAN);
			ly = 0;
		}
		else
			UpdateLine(ly);

		_curDot = 0;
		break;

	case Mode::OAM_SCAN:
		if (_curDot == oamScanDots) {
			SetMode(Mode::PIXEL_DRAW);
			break;
		}
		
		++_curDot;
		break;

	case Mode::PIXEL_DRAW:
		if (_curDot == _pixelDrawDots) {
			SetMode(Mode::HBLANK);
			break;
		}

		++_curDot;
		break;
	}

	return State::PROCESSING;
}

void GContext::UpdateLine(byte& ly) {
	++ly;

	bool lycEqLy = (ly == _memory[0xFF45]);
	byte& stat = _memory[0xFF41];

	// LCDStatus::LycEqLy should only be set here
	if (lycEqLy) {
		LCDStatus statFlags = static_cast<LCDStatus>(stat);
		statFlags.flags.LycEqLy = 1;

		if (statFlags.flags.LycIntSelect == 1)
			_memory.GetInterruptFlag().flags.LCDInt = 1;

		stat = statFlags;
	}
	else
		stat &= ~(1 << 2); // LycEqLy register = 0
}

Mode GContext::GetMode() const {
	return static_cast<Mode>(_memory.GetPPUMode());
}

void GContext::SetMode(Mode newMode) {
	byte& stat = _memory[0xFF41];
	
	stat &= ~3; // clear bits first
	stat |= static_cast<byte>(newMode); // then set to new value

	if (newMode == Mode::VBLANK)
		_memory.GetInterruptFlag().flags.VBlankInt = 1;

	if (newMode != Mode::PIXEL_DRAW) {
		byte statIntBit = (1 << 3) << static_cast<byte>(newMode);

		if ((stat & statIntBit) != 0)
			_memory.GetInterruptFlag().flags.LCDInt = 1; // LCDInt == Stat Int
	}
}

PaletteData GContext::GetPaletteData(Palette palette) const {
	return static_cast<PaletteData>(_memory[0xFF47 + static_cast<byte>(palette)]);
}

void GContext::SetPaletteData(Palette palette, PaletteData newIndices) {
	_memory[0xFF47 + static_cast<byte>(palette)] = newIndices;
}

} // namespace gb::ppu
